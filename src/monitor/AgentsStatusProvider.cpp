
#include <iostream>

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

#include <cpp_restapi/qt_connection.hpp>

#include "AgentsStatusProvider.hpp"
#include "common/JsonSerialize.h"
#include "common/constants.hpp"


AgentsStatusProvider::AgentsStatusProvider(QObject* parent)
    : IAgentsStatusProvider()
{
    Q_UNUSED(parent);
    m_watchdog.setInterval(WatchdogIntervalMs);
    connect(&m_watchdog, &QTimer::timeout, this, &AgentsStatusProvider::checkConnections);
    m_watchdog.start();
}


void AgentsStatusProvider::observe(const AgentInformation& info)
{
    if (m_connections.contains(info))
        return;

    const std::string address = QStringLiteral("http://%1:%2")
        .arg(info.host().toString())
        .arg(info.port())
        .toStdString();

    AgentConnection agentConn;
    agentConn.connection = std::make_shared<cpp_restapi::QtBackend::Connection>(
        m_nam, address, std::map<std::string, std::string>{});

    m_connections.insert(info, std::move(agentConn));

    // Fetch initial status, then trigger refresh, then connect SSE
    fetchInitialStatus(info);
}


void AgentsStatusProvider::unobserve(const AgentInformation& info)
{
    auto it = m_connections.find(info);
    if (it == m_connections.end())
        return;

    if (it->sseConnection)
        it->sseConnection->close();

    m_connections.erase(it);
}


void AgentsStatusProvider::fetchInitialStatus(const AgentInformation& info)
{
    emit connectionStateChanged(info, ConnectionState::Connecting);

    const QUrl url = QStringLiteral("http://%1:%2/api/v1/refresh")
                         .arg(info.host().toString())
                         .arg(info.port());

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply* reply = m_nam.post(req, QByteArray());

    QObject::connect(reply, &QNetworkReply::finished, this, [this, info, reply]() {
        reply->deleteLater();

        if (reply->error() == QNetworkReply::NoError)
        {
            parseStatusJson(info, reply->readAll());
            emit connectionStateChanged(info, ConnectionState::Connected);
        }
        else
        {
            std::cerr << "Failed to fetch status from " << info.name().toStdString()
                      << ": " << reply->errorString().toStdString() << "\n";
            emit connectionStateChanged(info, ConnectionState::Error);
        }

        connectSse(info);
        fetchAgentInfo(info);
    });
}


void AgentsStatusProvider::parseStatusJson(const AgentInformation& info, const QByteArray& json)
{
    try
    {
        nlohmann::json j = nlohmann::json::parse(json.toStdString());

        if (j.contains("overallHealth"))
        {
            GeneralHealth::Health health = j.at("overallHealth").get<GeneralHealth::Health>();
            emit statusChanged(info, health);
        }

        if (j.contains("disks"))
        {
            std::vector<DiskInfo> disks = j.at("disks").get<std::vector<DiskInfo>>();
            emit diskCollectionChanged(info, disks);
        }

        if (j.contains("lastRefreshed"))
        {
            QString ts = QString::fromStdString(j.at("lastRefreshed").get<std::string>());
            emit lastRefreshedChanged(info, ts);
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON parse error for agent " << info.name().toStdString()
                  << ": " << e.what() << "\n";
    }
}


void AgentsStatusProvider::fetchAgentInfo(const AgentInformation& info)
{
    auto it = m_connections.find(info);
    if (it == m_connections.end() || !it->connection)
        return;

    const std::string url = it->connection->url() + "/api/v1/info";

    it->connection->fetch(url,
        [this, info](cpp_restapi::Response response)
        {
            try
            {
                nlohmann::json j = nlohmann::json::parse(response.body);
                if (j.contains("protocol"))
                {
                    const int agentVersion = j.at("protocol").get<int>();
                    const int monitorVersion = static_cast<int>(VersionOfProtocol);
                    if (agentVersion != monitorVersion)
                        emit protocolMismatch(info, agentVersion, monitorVersion);
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << "Failed to parse agent info from " << info.name().toStdString()
                          << ": " << e.what() << "\n";
            }
        });
}


void AgentsStatusProvider::connectSse(const AgentInformation& info)
{
    auto it = m_connections.find(info);
    if (it == m_connections.end() || !it->connection)
        return;

    it->sseConnection = it->connection->subscribe("api/v1/events",
        [this, info](const cpp_restapi::SseEvent& event) {
            handleSseEvent(info, event);
        });
}


void AgentsStatusProvider::handleSseEvent(const AgentInformation& info, const cpp_restapi::SseEvent& event)
{
    auto it = m_connections.find(info);
    if (it == m_connections.end())
        return;

    it->reconnectDelayMs = 1000;
    it->lastEventTime = std::chrono::steady_clock::now();
    it->connected = true;

    emit connectionStateChanged(info, ConnectionState::Connected);

    try
    {
        nlohmann::json j = nlohmann::json::parse(event.data);

        if (j.contains("overallHealth"))
        {
            GeneralHealth::Health health = j.at("overallHealth").get<GeneralHealth::Health>();
            emit statusChanged(info, health);
        }

        if (j.contains("disks"))
        {
            std::vector<DiskInfo> disks = j.at("disks").get<std::vector<DiskInfo>>();
            emit diskCollectionChanged(info, disks);
        }

        if (j.contains("lastRefreshed"))
        {
            QString ts = QString::fromStdString(j.at("lastRefreshed").get<std::string>());
            emit lastRefreshedChanged(info, ts);
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON parse error for agent " << info.name().toStdString()
                  << ": " << e.what() << "\n";
    }
}


void AgentsStatusProvider::scheduleSseReconnect(const AgentInformation& info)
{
    auto it = m_connections.find(info);
    if (it == m_connections.end())
        return;

    const int delay = it->reconnectDelayMs;
    it->reconnectDelayMs = std::min(it->reconnectDelayMs * 2, MaxReconnectDelayMs);

    QTimer::singleShot(delay, this, [this, info]() {
        if (m_connections.contains(info))
            connectSse(info);
    });
}


void AgentsStatusProvider::checkConnections()
{
    const auto now = std::chrono::steady_clock::now();

    for (auto it = m_connections.begin(); it != m_connections.end(); ++it)
    {
        if (it->connected && (now - it->lastEventTime) > WatchdogTimeoutS)
        {
            it->connected = false;

            if (it->sseConnection)
                it->sseConnection->close();

            emit connectionStateChanged(it.key(), ConnectionState::Disconnected);
            scheduleSseReconnect(it.key());
        }
    }
}
