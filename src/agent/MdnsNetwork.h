#pragma once

#ifdef _WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

#include <chrono>
#include <string>
#include <vector>


struct MdnsNetworkAddresses
{
    sockaddr_in ipv4{};
    sockaddr_in6 ipv6{};
    bool hasIpv4 = false;
    bool hasIpv6 = false;
};

std::string getLocalHostname();
MdnsNetworkAddresses getLocalAddresses();
std::vector<int> openMdnsServiceSockets();
std::vector<int> waitForReadableSockets(const std::vector<int>& sockets,
                                        std::chrono::microseconds timeout);
