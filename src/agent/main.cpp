#include <algorithm>
#include <csignal>
#include <cstring>
#include <iostream>
#include <string>

#include "common/constants.hpp"
#include "HttpServer.h"
#include "MdnsPublisher.h"
#include "SystemUtilitiesFactory.h"
#include "DiscStatusCalculator.h"


namespace
{
    HttpServer* g_server = nullptr;

    void signalHandler(int)
    {
        if (g_server)
            g_server->stop();
    }
}


void collectAndPublish(HttpServer& server)
{
    SystemUtilitiesFactory factory;
    auto diskCollector = factory.diskCollector();
    auto diskCollection = diskCollector->GetDisksList();
    const auto probes = factory.getProbes();

    DiscStatusCalculator calc;
    std::vector<DiskInfo> diskInfos;

    for (const auto& disk : diskCollection)
    {
        std::vector<ProbeStatus> probeStatuses;
        probeStatuses.reserve(probes.size());

        for (const auto& probe : probes)
        {
            ProbeStatus status;
            status.health = probe->GetStatus(disk);
            status.rawData = probe->GetRawData(disk);
            probeStatuses.push_back(status);
        }

        DiskInfo info;
        info.SetName(disk.GetDeviceId());
        info.SetHealth(calc.CalculateDiskStatus(disk, probes));
        info.SetProbesStatuses(probeStatuses);
        diskInfos.push_back(info);
    }

    std::vector<GeneralHealth::Health> statuses;
    std::transform(diskInfos.begin(), diskInfos.end(), std::back_inserter(statuses),
                   [](const auto& di) { return di.GetHealth(); });

    auto overall = calc.CalculateCumulativeStatus(statuses);
    server.setStatusData(overall, std::move(diskInfos));
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

    std::cout << "Found disks:\n";
    for (const auto& disk : disks)
        std::cout << "  " << disk.GetDeviceId() << '\n';

    // Create HTTP server
    HttpServer server(agentName, RDHMPort);

    server.setRefreshCallback([&server] {
        collectAndPublish(server);
    });

    // Initial data collection
    collectAndPublish(server);

    // Start mDNS publisher
    MdnsPublisher mdns(agentName, ZeroConfServiceName, RDHMPort);
    mdns.start();

    // Setup signal handling for graceful shutdown
    g_server = &server;
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::cout << "Agent '" << agentName << "' listening on 0.0.0.0:" << RDHMPort << "\n";

    // Blocking — runs the HTTP server
    server.listen();

    // Cleanup
    mdns.stop();
    g_server = nullptr;

    std::cout << "Agent stopped.\n";
    return 0;
}
