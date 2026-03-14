#include "HttpServer.h"

#include <httplib.h>
#include <nlohmann/json.hpp>

#include "common/JsonSerialize.h"
#include "common/constants.hpp"

#include <atomic>
#include <chrono>
#include <iomanip>
#include <list>
#include <condition_variable>
#include <sstream>
#include <thread>


struct HttpServer::Impl
{
    httplib::Server server;
    std::string agentName;
    unsigned int port;

    std::mutex dataMutex;
    GeneralHealth::Health overallHealth = GeneralHealth::UNKNOWN;
    std::vector<DiskInfo> disks;
    std::string lastRefreshed;

    std::function<void()> refreshCallback;

    // Refresh cooldown
    static constexpr int RefreshCooldownSeconds = 600;  // 10 minutes
    std::chrono::steady_clock::time_point lastRefreshTime{};
    std::mutex refreshMutex;

    // SSE support
    static constexpr size_t MaxSseClients = 16;
    struct SseClient
    {
        std::mutex mutex;
        std::condition_variable cv;
        std::vector<std::string> pendingEvents;
        std::atomic<bool> alive{true};
    };

    std::mutex sseMutex;
    std::list<std::shared_ptr<SseClient>> sseClients;

    nlohmann::json buildStatusJson()
    {
        std::lock_guard lock(dataMutex);
        return nlohmann::json{
            {"overallHealth", overallHealth},
            {"disks", disks},
            {"lastRefreshed", lastRefreshed}
        };
    }

    void pushSseEvent(const std::string& event, const std::string& data)
    {
        std::lock_guard lock(sseMutex);
        for (auto it = sseClients.begin(); it != sseClients.end(); )
        {
            auto& client = *it;
            if (!client->alive)
            {
                it = sseClients.erase(it);
                continue;
            }

            {
                std::lock_guard cl(client->mutex);
                client->pendingEvents.push_back(
                    "event: " + event + "\ndata: " + data + "\n\n");
                client->cv.notify_one();
            }
            ++it;
        }
    }
};


HttpServer::HttpServer(const std::string& agentName, unsigned int port)
    : m_impl(std::make_unique<Impl>())
{
    m_impl->agentName = agentName;
    m_impl->port = port;

    // GET /api/v1/info
    m_impl->server.Get("/api/v1/info", [this](const httplib::Request&, httplib::Response& res) {
        nlohmann::json j = {
            {"name", m_impl->agentName},
            {"version", "1.0.0"},
            {"protocol", static_cast<int>(VersionOfProtocol)}
        };
        res.set_content(j.dump(), "application/json");
    });

    // GET /api/v1/status
    m_impl->server.Get("/api/v1/status", [this](const httplib::Request&, httplib::Response& res) {
        res.set_content(m_impl->buildStatusJson().dump(), "application/json");
    });

    // GET /api/v1/disks
    m_impl->server.Get("/api/v1/disks", [this](const httplib::Request&, httplib::Response& res) {
        std::lock_guard lock(m_impl->dataMutex);
        nlohmann::json j = m_impl->disks;
        res.set_content(j.dump(), "application/json");
    });

    // GET /api/v1/disks/:name
    m_impl->server.Get("/api/v1/disks/:name", [this](const httplib::Request& req, httplib::Response& res) {
        const auto& name = req.path_params.at("name");
        std::lock_guard lock(m_impl->dataMutex);

        for (const auto& d : m_impl->disks)
        {
            if (d.GetName() == name)
            {
                nlohmann::json j = d;
                res.set_content(j.dump(), "application/json");
                return;
            }
        }
        res.status = 404;
        res.set_content(R"({"error":"disk not found"})", "application/json");
    });

    // POST /api/v1/refresh
    m_impl->server.Post("/api/v1/refresh", [this](const httplib::Request&, httplib::Response& res) {
        bool refreshed = false;
        if (m_impl->refreshCallback)
        {
            std::lock_guard lock(m_impl->refreshMutex);
            const auto now = std::chrono::steady_clock::now();
            const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                now - m_impl->lastRefreshTime).count();

            if (elapsed >= Impl::RefreshCooldownSeconds)
            {
                m_impl->refreshCallback();
                m_impl->lastRefreshTime = now;
                refreshed = true;
            }
        }

        auto j = m_impl->buildStatusJson();
        j["refreshed"] = refreshed;
        res.set_content(j.dump(), "application/json");
    });

    // GET /api/v1/events — Server-Sent Events
    m_impl->server.Get("/api/v1/events", [this](const httplib::Request&, httplib::Response& res) {
        auto client = std::make_shared<Impl::SseClient>();

        {
            std::lock_guard lock(m_impl->sseMutex);

            // Remove dead clients first
            m_impl->sseClients.remove_if([](const auto& c) { return !c->alive.load(); });

            if (m_impl->sseClients.size() >= Impl::MaxSseClients)
            {
                res.status = 429;
                res.set_content(R"({"error":"too many SSE clients"})", "application/json");
                return;
            }

            m_impl->sseClients.push_back(client);
        }

        res.set_header("Cache-Control", "no-cache");
        res.set_header("Connection", "keep-alive");

        res.set_content_provider(
            "text/event-stream",
            [client](size_t /*offset*/, httplib::DataSink& sink) {
                std::unique_lock lock(client->mutex);
                client->cv.wait(lock, [&client] {
                    return !client->pendingEvents.empty() || !client->alive;
                });

                if (!client->alive)
                    return false;

                for (const auto& event : client->pendingEvents)
                    sink.write(event.c_str(), event.size());

                client->pendingEvents.clear();
                return true;
            },
            [client](bool /*success*/) {
                client->alive = false;
                client->cv.notify_all();
            }
        );
    });
}


HttpServer::~HttpServer()
{
    stop();
}


void HttpServer::setStatusData(GeneralHealth::Health overallHealth, std::vector<DiskInfo> disks)
{
    {
        std::lock_guard lock(m_impl->dataMutex);
        m_impl->overallHealth = overallHealth;
        m_impl->disks = std::move(disks);

        const auto now = std::chrono::system_clock::now();
        const auto time_t = std::chrono::system_clock::to_time_t(now);
        std::ostringstream oss;
        oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
        m_impl->lastRefreshed = oss.str();
    }

    // Push SSE event
    m_impl->pushSseEvent("statusChanged", m_impl->buildStatusJson().dump());
}


void HttpServer::listen()
{
    m_impl->server.listen("0.0.0.0", static_cast<int>(m_impl->port));
}


void HttpServer::stop()
{
    // Kill all SSE clients
    {
        std::lock_guard lock(m_impl->sseMutex);
        for (auto& client : m_impl->sseClients)
        {
            client->alive = false;
            client->cv.notify_all();
        }
    }
    m_impl->server.stop();
}


void HttpServer::setRefreshCallback(std::function<void()> cb)
{
    m_impl->refreshCallback = std::move(cb);
}
