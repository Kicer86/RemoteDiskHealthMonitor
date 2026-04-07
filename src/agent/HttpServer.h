#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <memory>
#include <functional>

#include "common/GeneralHealth.h"
#include "common/DiskInfo.h"


class HttpServer
{
public:
    HttpServer(std::string agentName, unsigned int port);
    ~HttpServer();

    void setStatusData(GeneralHealth::Health overallHealth, std::vector<DiskInfo> disks);

    // Blocking — call from main thread
    void listen();
    void stop();

    // Called to trigger data refresh; the callback does collection and calls setStatusData
    void setRefreshCallback(std::function<void()> cb);

    // Called when a new SSE client connects
    void setOnClientConnectedCallback(std::function<void()> cb);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
