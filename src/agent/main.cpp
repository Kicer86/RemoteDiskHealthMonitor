#include <algorithm>
#include <csignal>
#include <cstring>
#include <iostream>
#include <string>

#include "common/constants.hpp"
#include "common/DiskSummary.h"
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
}


void collectAndPublish(HttpServer& server, SystemUtilitiesFactory& factory)
{
    auto diskCollector = factory.diskCollector();
    auto diskCollection = diskCollector->GetDisksList();
    const auto probes = factory.getProbes();

    // Refresh all probes before reading cached data
    for (const auto& probe : probes)
        probe->Refresh(diskCollection);

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
        info.SetSummary(buildSummary(disk, probeStatuses));
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

    server.setRefreshCallback([&server, &systemUtilsFactory] {
        collectAndPublish(server, systemUtilsFactory);
    });

    // Initial data collection
    collectAndPublish(server, systemUtilsFactory);

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
