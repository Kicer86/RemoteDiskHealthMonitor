#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <cstring>
#include <iostream>
#include <mutex>
#include <ranges>
#include <string>
#include <thread>

#include "common/constants.hpp"
#include "common/DiskSummary.h"
#include "common/Utils.h"
#include "HttpServer.h"
#include "MdnsPublisher.h"
#include "SystemUtilitiesFactory.h"
#include "DiscStatusCalculator.h"


namespace
{
    std::atomic<HttpServer*> g_server{nullptr};
    std::atomic<bool> g_running{true};
    std::mutex g_bgMutex;
    std::condition_variable g_bgCv;

    void signalHandler(int)
    {
        g_running = false;
        if (auto* srv = g_server.load(std::memory_order_relaxed))
            srv->stop();
    }

    struct ProbeEntry
    {
        std::unique_ptr<IProbe> probe;
        std::chrono::steady_clock::time_point lastRefresh{};
    };

    DiskSummary buildSummary(const Disk& disk, const std::vector<ProbeStatus>& probeStatuses)
    {
        DiskSummary summary;
        summary.model = disk.GetModel();
        summary.vendor = disk.GetVendor();
        summary.capacityBytes = disk.GetCapacity();
        summary.driveType = disk.GetDriveType();

        // Extract temperature, power-on hours, and self-test status from SMART probe data
        for (const auto& probe : probeStatuses)
        {
            if (!probe.rawData.contains("type") || probe.rawData["type"] != "smart")
                continue;

            if (probe.rawData.contains("attributes"))
            {
                for (const auto& attr : probe.rawData["attributes"])
                {
                    const auto& name = attr.value("name", std::string{});
                    const auto id = attr.value("id", 0);

                    // Temperature: ATA id 194 or 190, NVMe by name
                    if (!summary.temperatureC &&
                        (id == 194 || id == 190 || name == "Temperature" ||
                         name == "Temperature_Celsius" || name == "Airflow_Temperature_Cel"))
                    {
                        summary.temperatureC = static_cast<int>(attr.value("rawVal", int64_t{0}));
                    }

                    // Power-On Hours: ATA id 9, NVMe by name
                    if (!summary.powerOnHours &&
                        (id == 9 || name == "Power_On_Hours"))
                    {
                        summary.powerOnHours = attr.value("rawVal", int64_t{0});
                    }
                }
            }

            if (probe.rawData.contains("selfTestStatus"))
            {
                const auto& st = probe.rawData["selfTestStatus"];
                SmartTestStatus testStatus;
                testStatus.running = st.value("running", false);
                testStatus.percentRemaining = st.value("percentRemaining", 0);
                testStatus.lastResult = st.value("lastResult", std::string{});
                summary.selfTestStatus = testStatus;
            }

            break;  // only need the first SMART probe
        }

        return summary;
    }

    // Returns true if any probe was refreshed
    bool refreshStaleProbes(std::vector<ProbeEntry>& entries,
                            const std::vector<Disk>& disks,
                            bool proactiveOnly = false)
    {
        bool anyRefreshed = false;
        const auto now = std::chrono::steady_clock::now();

        for (auto& entry : entries)
        {
            const auto policy = entry.probe->GetRefreshPolicy();

            if (proactiveOnly && !policy.proactiveCollection)
                continue;

            if ((now - entry.lastRefresh) >= policy.interval)
            {
                entry.probe->Refresh(disks);
                entry.lastRefresh = now;
                anyRefreshed = true;
            }
        }

        return anyRefreshed;
    }

    void refreshAllProbes(std::span<ProbeEntry> entries,
                          const std::vector<Disk>& disks)
    {
        const auto now = std::chrono::steady_clock::now();
        for (auto& entry : entries)
        {
            entry.probe->Refresh(disks);
            entry.lastRefresh = now;
        }
    }

    void publishFromCache(HttpServer& server,
                          const std::span<ProbeEntry> entries,
                          const std::vector<Disk>& disks)
    {
        DiscStatusCalculator calc;
        std::vector<DiskInfo> diskInfos;

        for (const auto& disk : disks)
        {
            std::vector<ProbeStatus> probeStatuses;
            std::vector<GeneralHealth::Health> healthStatuses;

            for (const auto& entry : entries)
            {
                probeStatuses.push_back({entry.probe->GetStatus(disk),
                                         entry.probe->GetRawData(disk)});
                healthStatuses.push_back(probeStatuses.back().health);
            }

            DiskInfo info;
            info.SetName(disk.GetDeviceId());
            info.SetHealth(calc.CalculateCumulativeStatus(healthStatuses));
            info.SetProbesStatuses(probeStatuses);
            info.SetSummary(buildSummary(disk, probeStatuses));
            diskInfos.push_back(info);
        }

        std::vector<GeneralHealth::Health> statuses;
        statuses.reserve(diskInfos.size());
        std::ranges::transform(diskInfos, std::back_inserter(statuses),
                               &DiskInfo::GetHealth);

        auto overall = calc.CalculateCumulativeStatus(statuses);
        server.setStatusData(overall, std::move(diskInfos));
    }

    // Lock hierarchy (acquire in this order to avoid deadlocks):
    //   refreshMutex (HttpServer::Impl) → g_probeMutex → dataMutex → sseMutex
    std::mutex g_probeMutex;
}


int main(int argc, char** argv)
{
    std::string agentName;

    // Simple arg parsing: -n <name> or --name <name>
    for (int i = 1; i < argc; ++i)
    {
        if ((std::strcmp(argv[i], "-n") == 0 || std::strcmp(argv[i], "--name") == 0) && i + 1 < argc)
        {
            agentName = argv[++i];
        }
        else if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0)
        {
            std::cout << "Usage: agent [-n|--name <name>]\n"
                      << "  -n, --name    Agent name visible via ZeroConf discovery\n";
            return 0;
        }
    }

    if (agentName.empty())
        agentName = "RDHAgent";

    // Enumerate disks
    SystemUtilitiesFactory systemUtilsFactory;
    auto diskCollector = systemUtilsFactory.diskCollector();
    const auto disks = diskCollector->GetDisksList();

    std::vector<std::vector<std::string>> disksInfo{
        {"ID", "type", "capacity", "vendor", "model"}
    };

    for (const auto& disk : disks)
        disksInfo.emplace_back(
        std::vector {
            disk.GetDeviceId(),
            disk.GetDriveType(),
            formatBytes(disk.GetCapacity()),
            disk.GetVendor(),
            disk.GetModel()
        });

    std::cout << "Found disks:\n" << formatTable(disksInfo);

    // Create persistent probes
    auto probeUptrs = systemUtilsFactory.getProbes();
    auto probeEntries = std::ranges::to<std::vector<ProbeEntry>>(std::views::transform(probeUptrs, [](auto&& probe) { return ProbeEntry(std::move(probe)); }));

    // Create HTTP server
    HttpServer server(agentName, RDHMPort);

    // POST /api/v1/refresh — force refresh all probes (manual trigger)
    server.setRefreshCallback([&server, &probeEntries, &disks] {
        std::lock_guard lock(g_probeMutex);
        refreshAllProbes(probeEntries, disks);
        publishFromCache(server, probeEntries, disks);
    });

    // SSE client connected — immediately send cached state, then wake
    // background thread so stale probes get refreshed asynchronously
    server.setOnClientConnectedCallback([&server, &probeEntries, &disks] {
        {
            std::lock_guard lock(g_probeMutex);
            publishFromCache(server, probeEntries, disks);
        }
        g_bgCv.notify_one();
    });

    // Initial proactive data collection
    {
        std::lock_guard lock(g_probeMutex);
        refreshStaleProbes(probeEntries, disks, true);
        publishFromCache(server, probeEntries, disks);
    }

    // Start mDNS publisher
    MdnsPublisher mdns(agentName, ZeroConfServiceName, RDHMPort);
    mdns.start();

    // Setup signal handling for graceful shutdown
    g_server = &server;
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::cout << "Agent '" << agentName << "' listening on 0.0.0.0:" << RDHMPort << "\n";

    // Background thread for periodic and on-demand stale-probe refresh
    std::thread bgThread([&server, &probeEntries, &disks] {
        while (g_running)
        {
            {
                std::unique_lock lock(g_bgMutex);
                g_bgCv.wait_for(lock, std::chrono::minutes(1),
                                [] { return !g_running.load(); });
            }

            if (!g_running)
                break;

            std::lock_guard lock(g_probeMutex);
            if (refreshStaleProbes(probeEntries, disks))
                publishFromCache(server, probeEntries, disks);
        }
    });

    // Blocking — runs the HTTP server
    server.listen();

    // Cleanup
    g_running = false;
    g_bgCv.notify_all();
    bgThread.join();

    mdns.stop();
    g_server = nullptr;

    std::cout << "Agent stopped.\n";
    return 0;
}
