#include "MdnsPublisher.h"

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS 1
#include <winsock2.h>
#include <iphlpapi.h>
#else
#include <netdb.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/time.h>
#endif

#include <mdns.h>

#include <atomic>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>


namespace
{

struct ServiceData
{
    mdns_string_t service;
    mdns_string_t hostname;
    mdns_string_t service_instance;
    mdns_string_t hostname_qualified;
    struct sockaddr_in address_ipv4;
    struct sockaddr_in6 address_ipv6;
    int port;
    mdns_record_t record_ptr;
    mdns_record_t record_srv;
    mdns_record_t record_a;
    mdns_record_t record_aaaa;
    mdns_record_t txt_record;
};

// Get local network addresses
void getLocalAddresses(struct sockaddr_in& addr_ipv4, struct sockaddr_in6& addr_ipv6,
                       bool& has_ipv4, bool& has_ipv6)
{
    memset(&addr_ipv4, 0, sizeof(addr_ipv4));
    memset(&addr_ipv6, 0, sizeof(addr_ipv6));
    has_ipv4 = false;
    has_ipv6 = false;

#ifdef _WIN32
    IP_ADAPTER_ADDRESSES* adapter_address = nullptr;
    ULONG address_size = 8000;
    unsigned int ret;
    unsigned int num_retries = 4;
    do {
        adapter_address = (IP_ADAPTER_ADDRESSES*)malloc(address_size);
        ret = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_ANYCAST, 0,
                                   adapter_address, &address_size);
        if (ret == ERROR_BUFFER_OVERFLOW) {
            free(adapter_address);
            adapter_address = nullptr;
            address_size *= 2;
        } else {
            break;
        }
    } while (num_retries-- > 0);

    if (!adapter_address || (ret != NO_ERROR)) {
        free(adapter_address);
        return;
    }

    for (PIP_ADAPTER_ADDRESSES adapter = adapter_address; adapter; adapter = adapter->Next) {
        if (adapter->TunnelType == TUNNEL_TYPE_TEREDO)
            continue;
        if (adapter->OperStatus != IfOperStatusUp)
            continue;

        for (IP_ADAPTER_UNICAST_ADDRESS* unicast = adapter->FirstUnicastAddress; unicast;
             unicast = unicast->Next) {
            if (unicast->Address.lpSockaddr->sa_family == AF_INET && !has_ipv4) {
                addr_ipv4 = *(struct sockaddr_in*)unicast->Address.lpSockaddr;
                has_ipv4 = true;
            } else if (unicast->Address.lpSockaddr->sa_family == AF_INET6 && !has_ipv6) {
                auto* saddr = (struct sockaddr_in6*)unicast->Address.lpSockaddr;
                if (saddr->sin6_scope_id == 0) {
                    addr_ipv6 = *saddr;
                    has_ipv6 = true;
                }
            }
        }
    }
    free(adapter_address);
#else
    struct ifaddrs* ifaddr = nullptr;
    if (getifaddrs(&ifaddr) < 0)
        return;

    for (struct ifaddrs* ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr)
            continue;
        if (!(ifa->ifa_flags & IFF_UP) || !(ifa->ifa_flags & IFF_MULTICAST))
            continue;
        if ((ifa->ifa_flags & IFF_LOOPBACK) || (ifa->ifa_flags & IFF_POINTOPOINT))
            continue;

        if (ifa->ifa_addr->sa_family == AF_INET && !has_ipv4) {
            auto* saddr = (struct sockaddr_in*)ifa->ifa_addr;
            if (saddr->sin_addr.s_addr != htonl(INADDR_LOOPBACK)) {
                addr_ipv4 = *saddr;
                has_ipv4 = true;
            }
        } else if (ifa->ifa_addr->sa_family == AF_INET6 && !has_ipv6) {
            auto* saddr = (struct sockaddr_in6*)ifa->ifa_addr;
            if (saddr->sin6_scope_id)
                continue;
            static const unsigned char localhost[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
            if (memcmp(saddr->sin6_addr.s6_addr, localhost, 16) != 0) {
                addr_ipv6 = *saddr;
                has_ipv6 = true;
            }
        }
    }
    freeifaddrs(ifaddr);
#endif
}

int openServiceSockets(int* sockets, int maxSockets,
                       struct sockaddr_in& addr_ipv4, struct sockaddr_in6& addr_ipv6,
                       bool& has_ipv4, bool& has_ipv6)
{
    int num_sockets = 0;

    getLocalAddresses(addr_ipv4, addr_ipv6, has_ipv4, has_ipv6);

    if (num_sockets < maxSockets) {
        struct sockaddr_in sock_addr;
        memset(&sock_addr, 0, sizeof(sock_addr));
        sock_addr.sin_family = AF_INET;
#ifdef _WIN32
        sock_addr.sin_addr = in4addr_any;
#else
        sock_addr.sin_addr.s_addr = INADDR_ANY;
#endif
        sock_addr.sin_port = htons(MDNS_PORT);
        int sock = mdns_socket_open_ipv4(&sock_addr);
        if (sock >= 0)
            sockets[num_sockets++] = sock;
    }

    if (num_sockets < maxSockets) {
        struct sockaddr_in6 sock_addr;
        memset(&sock_addr, 0, sizeof(sock_addr));
        sock_addr.sin6_family = AF_INET6;
        sock_addr.sin6_addr = in6addr_any;
        sock_addr.sin6_port = htons(MDNS_PORT);
        int sock = mdns_socket_open_ipv6(&sock_addr);
        if (sock >= 0)
            sockets[num_sockets++] = sock;
    }

    return num_sockets;
}


int serviceCallback(int sock, const struct sockaddr* from, size_t addrlen,
                    mdns_entry_type_t entry, uint16_t query_id, uint16_t rtype,
                    uint16_t rclass, uint32_t /*ttl*/, const void* data,
                    size_t size, size_t name_offset, size_t /*name_length*/,
                    size_t /*record_offset*/, size_t /*record_length*/, void* user_data)
{
    if (entry != MDNS_ENTRYTYPE_QUESTION)
        return 0;

    const char dns_sd[] = "_services._dns-sd._udp.local.";
    const auto* service = static_cast<const ServiceData*>(user_data);

    char namebuffer[256];
    size_t offset = name_offset;
    mdns_string_t name = mdns_string_extract(data, size, &offset, namebuffer, sizeof(namebuffer));

    char sendbuffer[1024];
    const uint16_t unicast = (rclass & MDNS_UNICAST_RESPONSE);

    if ((name.length == (sizeof(dns_sd) - 1)) &&
        (strncmp(name.str, dns_sd, sizeof(dns_sd) - 1) == 0))
    {
        if ((rtype == MDNS_RECORDTYPE_PTR) || (rtype == MDNS_RECORDTYPE_ANY))
        {
            mdns_record_t answer = {};
            answer.name = name;
            answer.type = MDNS_RECORDTYPE_PTR;
            answer.data.ptr.name = service->service;

            if (unicast) {
                mdns_query_answer_unicast(sock, from, addrlen, sendbuffer, sizeof(sendbuffer),
                                          query_id, static_cast<mdns_record_type_t>(rtype), name.str, name.length, answer, 0, 0, 0, 0);
            } else {
                mdns_query_answer_multicast(sock, sendbuffer, sizeof(sendbuffer), answer, 0, 0, 0, 0);
            }
        }
    }
    else if ((name.length == service->service.length) &&
             (strncmp(name.str, service->service.str, name.length) == 0))
    {
        if ((rtype == MDNS_RECORDTYPE_PTR) || (rtype == MDNS_RECORDTYPE_ANY))
        {
            mdns_record_t answer = service->record_ptr;

            mdns_record_t additional[5] = {};
            size_t additional_count = 0;
            additional[additional_count++] = service->record_srv;
            if (service->address_ipv4.sin_family == AF_INET)
                additional[additional_count++] = service->record_a;
            if (service->address_ipv6.sin6_family == AF_INET6)
                additional[additional_count++] = service->record_aaaa;
            additional[additional_count++] = service->txt_record;

            if (unicast) {
                mdns_query_answer_unicast(sock, from, addrlen, sendbuffer, sizeof(sendbuffer),
                                          query_id, static_cast<mdns_record_type_t>(rtype), name.str, name.length, answer, 0, 0,
                                          additional, additional_count);
            } else {
                mdns_query_answer_multicast(sock, sendbuffer, sizeof(sendbuffer), answer, 0, 0,
                                            additional, additional_count);
            }
        }
    }
    else if ((name.length == service->service_instance.length) &&
             (strncmp(name.str, service->service_instance.str, name.length) == 0))
    {
        if ((rtype == MDNS_RECORDTYPE_SRV) || (rtype == MDNS_RECORDTYPE_ANY))
        {
            mdns_record_t answer = service->record_srv;

            mdns_record_t additional[5] = {};
            size_t additional_count = 0;
            if (service->address_ipv4.sin_family == AF_INET)
                additional[additional_count++] = service->record_a;
            if (service->address_ipv6.sin6_family == AF_INET6)
                additional[additional_count++] = service->record_aaaa;
            additional[additional_count++] = service->txt_record;

            if (unicast) {
                mdns_query_answer_unicast(sock, from, addrlen, sendbuffer, sizeof(sendbuffer),
                                          query_id, static_cast<mdns_record_type_t>(rtype), name.str, name.length, answer, 0, 0,
                                          additional, additional_count);
            } else {
                mdns_query_answer_multicast(sock, sendbuffer, sizeof(sendbuffer), answer, 0, 0,
                                            additional, additional_count);
            }
        }
    }
    else if ((name.length == service->hostname_qualified.length) &&
             (strncmp(name.str, service->hostname_qualified.str, name.length) == 0))
    {
        if (((rtype == MDNS_RECORDTYPE_A) || (rtype == MDNS_RECORDTYPE_ANY)) &&
            (service->address_ipv4.sin_family == AF_INET))
        {
            mdns_record_t answer = service->record_a;

            mdns_record_t additional[5] = {};
            size_t additional_count = 0;
            if (service->address_ipv6.sin6_family == AF_INET6)
                additional[additional_count++] = service->record_aaaa;
            additional[additional_count++] = service->txt_record;

            if (unicast) {
                mdns_query_answer_unicast(sock, from, addrlen, sendbuffer, sizeof(sendbuffer),
                                          query_id, static_cast<mdns_record_type_t>(rtype), name.str, name.length, answer, 0, 0,
                                          additional, additional_count);
            } else {
                mdns_query_answer_multicast(sock, sendbuffer, sizeof(sendbuffer), answer, 0, 0,
                                            additional, additional_count);
            }
        }
        else if (((rtype == MDNS_RECORDTYPE_AAAA) || (rtype == MDNS_RECORDTYPE_ANY)) &&
                 (service->address_ipv6.sin6_family == AF_INET6))
        {
            mdns_record_t answer = service->record_aaaa;

            mdns_record_t additional[5] = {};
            size_t additional_count = 0;
            if (service->address_ipv4.sin_family == AF_INET)
                additional[additional_count++] = service->record_a;
            additional[additional_count++] = service->txt_record;

            if (unicast) {
                mdns_query_answer_unicast(sock, from, addrlen, sendbuffer, sizeof(sendbuffer),
                                          query_id, static_cast<mdns_record_type_t>(rtype), name.str, name.length, answer, 0, 0,
                                          additional, additional_count);
            } else {
                mdns_query_answer_multicast(sock, sendbuffer, sizeof(sendbuffer), answer, 0, 0,
                                            additional, additional_count);
            }
        }
    }

    return 0;
}

} // anonymous namespace


struct MdnsPublisher::Impl
{
    std::string serviceName;     // e.g. "MyAgent"
    std::string serviceType;     // e.g. "_RDHMonitor._tcp"
    unsigned int port;

    // String buffers must outlive the mdns_string_t pointers
    std::string serviceTypeLocal;        // "_RDHMonitor._tcp.local."
    std::string serviceInstanceLocal;    // "MyAgent._RDHMonitor._tcp.local."
    std::string hostnameLocal;           // "myhostname.local."

    ServiceData service;

    int sockets[32] = {};
    int numSockets = 0;

    std::atomic<bool> running{false};
    std::thread listenThread;
};


MdnsPublisher::MdnsPublisher(const std::string& serviceName, const std::string& serviceType,
                             unsigned int port)
    : m_impl(std::make_unique<Impl>())
{
    m_impl->serviceName = serviceName;
    m_impl->serviceType = serviceType;
    m_impl->port = port;
}


MdnsPublisher::~MdnsPublisher()
{
    stop();
}


void MdnsPublisher::start()
{
    if (m_impl->running)
        return;

    // Prepare string buffers
    m_impl->serviceTypeLocal = m_impl->serviceType + ".local.";
    m_impl->serviceInstanceLocal = m_impl->serviceName + "." + m_impl->serviceType + ".local.";

    char hostnameBuffer[256] = {};
    gethostname(hostnameBuffer, sizeof(hostnameBuffer));
    m_impl->hostnameLocal = std::string(hostnameBuffer) + ".local.";

    // Get local addresses and open service sockets
    bool has_ipv4 = false, has_ipv6 = false;
    struct sockaddr_in addr_ipv4;
    struct sockaddr_in6 addr_ipv6;

    m_impl->numSockets = openServiceSockets(m_impl->sockets, 32,
                                            addr_ipv4, addr_ipv6, has_ipv4, has_ipv6);

    if (m_impl->numSockets <= 0) {
        std::cerr << "MdnsPublisher: failed to open mDNS sockets\n";
        return;
    }

    // Build service data
    auto& svc = m_impl->service;
    memset(&svc, 0, sizeof(svc));

    svc.service = {m_impl->serviceTypeLocal.c_str(), m_impl->serviceTypeLocal.size()};
    svc.hostname = {m_impl->serviceName.c_str(), m_impl->serviceName.size()};
    svc.service_instance = {m_impl->serviceInstanceLocal.c_str(), m_impl->serviceInstanceLocal.size()};
    svc.hostname_qualified = {m_impl->hostnameLocal.c_str(), m_impl->hostnameLocal.size()};
    svc.address_ipv4 = has_ipv4 ? addr_ipv4 : sockaddr_in{};
    svc.address_ipv6 = has_ipv6 ? addr_ipv6 : sockaddr_in6{};
    svc.port = static_cast<int>(m_impl->port);

    // PTR: "_service._tcp.local." -> "MyAgent._service._tcp.local."
    svc.record_ptr.name = svc.service;
    svc.record_ptr.type = MDNS_RECORDTYPE_PTR;
    svc.record_ptr.data.ptr.name = svc.service_instance;

    // SRV: "MyAgent._service._tcp.local." -> "hostname.local." + port
    svc.record_srv.name = svc.service_instance;
    svc.record_srv.type = MDNS_RECORDTYPE_SRV;
    svc.record_srv.data.srv.name = svc.hostname_qualified;
    svc.record_srv.data.srv.port = static_cast<uint16_t>(svc.port);
    svc.record_srv.data.srv.priority = 0;
    svc.record_srv.data.srv.weight = 0;

    // A record
    svc.record_a.name = svc.hostname_qualified;
    svc.record_a.type = MDNS_RECORDTYPE_A;
    svc.record_a.data.a.addr = svc.address_ipv4;

    // AAAA record
    svc.record_aaaa.name = svc.hostname_qualified;
    svc.record_aaaa.type = MDNS_RECORDTYPE_AAAA;
    svc.record_aaaa.data.aaaa.addr = svc.address_ipv6;

    // TXT record
    svc.txt_record.name = svc.service_instance;
    svc.txt_record.type = MDNS_RECORDTYPE_TXT;
    svc.txt_record.data.txt.key = {MDNS_STRING_CONST("RDHAgent")};
    svc.txt_record.data.txt.value = {MDNS_STRING_CONST("1")};

    // Send initial announcement
    {
        char buffer[2048];
        mdns_record_t additional[5] = {};
        size_t additional_count = 0;
        additional[additional_count++] = svc.record_srv;
        if (has_ipv4)
            additional[additional_count++] = svc.record_a;
        if (has_ipv6)
            additional[additional_count++] = svc.record_aaaa;
        additional[additional_count++] = svc.txt_record;

        for (int i = 0; i < m_impl->numSockets; ++i)
            mdns_announce_multicast(m_impl->sockets[i], buffer, sizeof(buffer),
                                    svc.record_ptr, 0, 0, additional, additional_count);
    }

    std::cout << "mDNS service published: " << m_impl->serviceInstanceLocal << " on port " << m_impl->port << "\n";

    // Start listener thread
    m_impl->running = true;
    m_impl->listenThread = std::thread([this] {
        char buffer[2048];
        while (m_impl->running) {
            int nfds = 0;
            fd_set readfs;
            FD_ZERO(&readfs);
            for (int i = 0; i < m_impl->numSockets; ++i) {
                if (m_impl->sockets[i] >= nfds)
                    nfds = m_impl->sockets[i] + 1;
                FD_SET(m_impl->sockets[i], &readfs);
            }

            struct timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = 200000;  // 200ms poll

            if (select(nfds, &readfs, nullptr, nullptr, &timeout) >= 0) {
                for (int i = 0; i < m_impl->numSockets; ++i) {
                    if (FD_ISSET(m_impl->sockets[i], &readfs)) {
                        mdns_socket_listen(m_impl->sockets[i], buffer, sizeof(buffer),
                                           serviceCallback, &m_impl->service);
                    }
                }
            }
        }
    });
}


void MdnsPublisher::stop()
{
    if (!m_impl->running)
        return;

    m_impl->running = false;
    if (m_impl->listenThread.joinable())
        m_impl->listenThread.join();

    // Send goodbye
    {
        auto& svc = m_impl->service;
        char buffer[2048];
        mdns_record_t additional[5] = {};
        size_t additional_count = 0;
        additional[additional_count++] = svc.record_srv;
        if (svc.address_ipv4.sin_family == AF_INET)
            additional[additional_count++] = svc.record_a;
        if (svc.address_ipv6.sin6_family == AF_INET6)
            additional[additional_count++] = svc.record_aaaa;
        additional[additional_count++] = svc.txt_record;

        for (int i = 0; i < m_impl->numSockets; ++i)
            mdns_goodbye_multicast(m_impl->sockets[i], buffer, sizeof(buffer),
                                   svc.record_ptr, 0, 0, additional, additional_count);
    }

    // Close sockets
    for (int i = 0; i < m_impl->numSockets; ++i)
        mdns_socket_close(m_impl->sockets[i]);
    m_impl->numSockets = 0;

    std::cout << "mDNS service unpublished\n";
}
