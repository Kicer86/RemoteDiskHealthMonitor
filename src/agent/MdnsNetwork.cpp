#include "MdnsNetwork.h"

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS 1
#include <iphlpapi.h>
#else
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/select.h>
#include <unistd.h>
#endif

#include <mdns.h>

#include <cstdlib>
#include <cstring>


namespace
{

constexpr auto HostnameFallback = "rdhm-agent";

#ifdef _WIN32
void collectLocalAddresses(MdnsNetworkAddresses& result)
{
    IP_ADAPTER_ADDRESSES* adapterAddress = nullptr;
    ULONG addressSize = 8000;
    ULONG ret = NO_ERROR;
    unsigned int retries = 4;

    do {
        adapterAddress = static_cast<IP_ADAPTER_ADDRESSES*>(malloc(addressSize));
        ret = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_ANYCAST,
                                   nullptr, adapterAddress, &addressSize);

        if (ret == ERROR_BUFFER_OVERFLOW) {
            free(adapterAddress);
            adapterAddress = nullptr;
            addressSize *= 2;
        } else {
            break;
        }
    } while (retries-- > 0);

    if (!adapterAddress || ret != NO_ERROR) {
        free(adapterAddress);
        return;
    }

    for (PIP_ADAPTER_ADDRESSES adapter = adapterAddress; adapter; adapter = adapter->Next) {
        if (adapter->TunnelType == TUNNEL_TYPE_TEREDO || adapter->OperStatus != IfOperStatusUp)
            continue;

        for (IP_ADAPTER_UNICAST_ADDRESS* unicast = adapter->FirstUnicastAddress; unicast;
             unicast = unicast->Next) {
            const auto* socketAddress = unicast->Address.lpSockaddr;

            if (socketAddress->sa_family == AF_INET && !result.hasIpv4) {
                result.ipv4 = *reinterpret_cast<const sockaddr_in*>(socketAddress);
                result.hasIpv4 = true;
            } else if (socketAddress->sa_family == AF_INET6 && !result.hasIpv6) {
                const auto* ipv6 = reinterpret_cast<const sockaddr_in6*>(socketAddress);
                if (ipv6->sin6_scope_id == 0) {
                    result.ipv6 = *ipv6;
                    result.hasIpv6 = true;
                }
            }
        }
    }

    free(adapterAddress);
}
#else
bool isUsableInterface(const ifaddrs& interface)
{
    return interface.ifa_addr &&
           (interface.ifa_flags & IFF_UP) &&
           (interface.ifa_flags & IFF_MULTICAST) &&
           !(interface.ifa_flags & IFF_LOOPBACK) &&
           !(interface.ifa_flags & IFF_POINTOPOINT);
}

bool isLoopbackAddress(const sockaddr_in& address)
{
    return address.sin_addr.s_addr == htonl(INADDR_LOOPBACK);
}

bool isLoopbackAddress(const sockaddr_in6& address)
{
    static const unsigned char localhost[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 1};
    return memcmp(address.sin6_addr.s6_addr, localhost, sizeof(localhost)) == 0;
}

void collectLocalAddresses(MdnsNetworkAddresses& result)
{
    ifaddrs* interfaces = nullptr;
    if (getifaddrs(&interfaces) < 0)
        return;

    for (ifaddrs* interface = interfaces; interface; interface = interface->ifa_next) {
        if (!isUsableInterface(*interface))
            continue;

        if (interface->ifa_addr->sa_family == AF_INET && !result.hasIpv4) {
            const auto* ipv4 = reinterpret_cast<const sockaddr_in*>(interface->ifa_addr);
            if (!isLoopbackAddress(*ipv4)) {
                result.ipv4 = *ipv4;
                result.hasIpv4 = true;
            }
        } else if (interface->ifa_addr->sa_family == AF_INET6 && !result.hasIpv6) {
            const auto* ipv6 = reinterpret_cast<const sockaddr_in6*>(interface->ifa_addr);
            if (ipv6->sin6_scope_id == 0 && !isLoopbackAddress(*ipv6)) {
                result.ipv6 = *ipv6;
                result.hasIpv6 = true;
            }
        }
    }

    freeifaddrs(interfaces);
}
#endif

void addSocketIfOpen(std::vector<int>& sockets, int socket)
{
    if (socket >= 0)
        sockets.push_back(socket);
}

timeval toTimeval(std::chrono::microseconds timeout)
{
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(timeout);
    const auto microseconds = timeout - seconds;

    timeval result{};
    result.tv_sec = seconds.count();
    result.tv_usec = microseconds.count();
    return result;
}

} // anonymous namespace


std::string getLocalHostname()
{
    char hostname[256] = {};
    if (gethostname(hostname, sizeof(hostname)) != 0 || hostname[0] == '\0')
        return HostnameFallback;

    hostname[sizeof(hostname) - 1] = '\0';
    return hostname;
}


MdnsNetworkAddresses getLocalAddresses()
{
    MdnsNetworkAddresses result;
    collectLocalAddresses(result);
    return result;
}


std::vector<int> openMdnsServiceSockets()
{
    std::vector<int> sockets;
    sockets.reserve(2);

    sockaddr_in ipv4{};
    ipv4.sin_family = AF_INET;
#ifdef _WIN32
    ipv4.sin_addr = in4addr_any;
#else
    ipv4.sin_addr.s_addr = INADDR_ANY;
#endif
    ipv4.sin_port = htons(MDNS_PORT);
    addSocketIfOpen(sockets, mdns_socket_open_ipv4(&ipv4));

    sockaddr_in6 ipv6{};
    ipv6.sin6_family = AF_INET6;
    ipv6.sin6_addr = in6addr_any;
    ipv6.sin6_port = htons(MDNS_PORT);
    addSocketIfOpen(sockets, mdns_socket_open_ipv6(&ipv6));

    return sockets;
}


std::vector<int> waitForReadableSockets(const std::vector<int>& sockets,
                                        std::chrono::microseconds timeout)
{
    std::vector<int> ready;

    if (!sockets.empty())
    {
        fd_set readable;
        FD_ZERO(&readable);

        int nfds = 0;
        for (int socket : sockets) {
            if (socket >= nfds)
                nfds = socket + 1;
            FD_SET(socket, &readable);
        }

        timeval tv = toTimeval(timeout);
        if (select(nfds, &readable, nullptr, nullptr, &tv) > 0)
        {
            ready.reserve(sockets.size());

            for (int socket : sockets) {
                if (FD_ISSET(socket, &readable))
                    ready.push_back(socket);
            }
        }
    }

    return ready;
}
