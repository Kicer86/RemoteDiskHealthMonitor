#include "MdnsPublisher.h"

#include "MdnsNetwork.h"

#include <mdns.h>

#include <array>
#include <atomic>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>


namespace
{

constexpr auto DnsSdServiceType = "_services._dns-sd._udp.local.";
constexpr auto ResponsePollTimeout = std::chrono::milliseconds(200);
constexpr size_t ResponseBufferSize = 2048;

struct ServiceData
{
    mdns_string_t service;
    mdns_string_t serviceInstance;
    mdns_string_t hostnameQualified;
    sockaddr_in addressIpv4;
    sockaddr_in6 addressIpv6;
    int port;
    mdns_record_t ptrRecord;
    mdns_record_t srvRecord;
    mdns_record_t aRecord;
    mdns_record_t aaaaRecord;
    mdns_record_t txtRecord;
};

struct RecordList
{
    std::array<mdns_record_t, 5> records{};
    size_t count = 0;

    void add(const mdns_record_t& record)
    {
        records[count++] = record;
    }
};

mdns_string_t mdnsString(const std::string& value)
{
    return {value.c_str(), value.size()};
}

bool hasIpv4Address(const ServiceData& service)
{
    return service.addressIpv4.sin_family == AF_INET;
}

bool hasIpv6Address(const ServiceData& service)
{
    return service.addressIpv6.sin6_family == AF_INET6;
}

bool sameName(mdns_string_t lhs, mdns_string_t rhs)
{
    return lhs.length == rhs.length && strncmp(lhs.str, rhs.str, lhs.length) == 0;
}

bool sameName(mdns_string_t lhs, const char* rhs)
{
    const size_t rhsLength = strlen(rhs);
    return lhs.length == rhsLength && strncmp(lhs.str, rhs, rhsLength) == 0;
}

bool acceptsRecordType(uint16_t requestedType, mdns_record_type_t availableType)
{
    return requestedType == availableType || requestedType == MDNS_RECORDTYPE_ANY;
}

RecordList serviceRecords(const ServiceData& service)
{
    RecordList records;
    records.add(service.srvRecord);

    if (hasIpv4Address(service))
        records.add(service.aRecord);
    if (hasIpv6Address(service))
        records.add(service.aaaaRecord);

    records.add(service.txtRecord);
    return records;
}

RecordList instanceRecords(const ServiceData& service)
{
    RecordList records;

    if (hasIpv4Address(service))
        records.add(service.aRecord);
    if (hasIpv6Address(service))
        records.add(service.aaaaRecord);

    records.add(service.txtRecord);
    return records;
}

RecordList addressRecords(const ServiceData& service, mdns_record_type_t answeredType)
{
    RecordList records;

    if (answeredType == MDNS_RECORDTYPE_A && hasIpv6Address(service))
        records.add(service.aaaaRecord);
    if (answeredType == MDNS_RECORDTYPE_AAAA && hasIpv4Address(service))
        records.add(service.aRecord);

    records.add(service.txtRecord);
    return records;
}

void answerQuestion(int socket, const sockaddr* from, size_t addressLength,
                    uint16_t queryId, uint16_t queryType, bool unicast,
                    mdns_string_t questionName, const mdns_record_t& answer,
                    const RecordList& additional)
{
    char buffer[1024] = {};
    const auto recordType = static_cast<mdns_record_type_t>(queryType);

    if (unicast) {
        mdns_query_answer_unicast(socket, from, addressLength, buffer, sizeof(buffer),
                                  queryId, recordType, questionName.str, questionName.length,
                                  answer, nullptr, 0, additional.records.data(), additional.count);
    } else {
        mdns_query_answer_multicast(socket, buffer, sizeof(buffer), answer, nullptr, 0,
                                    additional.records.data(), additional.count);
    }
}

ServiceData buildServiceData(const std::string& serviceTypeLocal,
                             const std::string& serviceInstanceLocal,
                             const std::string& hostnameLocal,
                             const MdnsNetworkAddresses& addresses,
                             unsigned int port)
{
    ServiceData service{};

    service.service = mdnsString(serviceTypeLocal);
    service.serviceInstance = mdnsString(serviceInstanceLocal);
    service.hostnameQualified = mdnsString(hostnameLocal);
    service.addressIpv4 = addresses.hasIpv4 ? addresses.ipv4 : sockaddr_in{};
    service.addressIpv6 = addresses.hasIpv6 ? addresses.ipv6 : sockaddr_in6{};
    service.port = static_cast<int>(port);

    service.ptrRecord.name = service.service;
    service.ptrRecord.type = MDNS_RECORDTYPE_PTR;
    service.ptrRecord.data.ptr.name = service.serviceInstance;

    service.srvRecord.name = service.serviceInstance;
    service.srvRecord.type = MDNS_RECORDTYPE_SRV;
    service.srvRecord.data.srv.name = service.hostnameQualified;
    service.srvRecord.data.srv.port = static_cast<uint16_t>(service.port);
    service.srvRecord.data.srv.priority = 0;
    service.srvRecord.data.srv.weight = 0;

    service.aRecord.name = service.hostnameQualified;
    service.aRecord.type = MDNS_RECORDTYPE_A;
    service.aRecord.data.a.addr = service.addressIpv4;

    service.aaaaRecord.name = service.hostnameQualified;
    service.aaaaRecord.type = MDNS_RECORDTYPE_AAAA;
    service.aaaaRecord.data.aaaa.addr = service.addressIpv6;

    service.txtRecord.name = service.serviceInstance;
    service.txtRecord.type = MDNS_RECORDTYPE_TXT;
    service.txtRecord.data.txt.key = {MDNS_STRING_CONST("RDHAgent")};
    service.txtRecord.data.txt.value = {MDNS_STRING_CONST("1")};

    return service;
}

void announceService(const std::vector<int>& sockets, const ServiceData& service)
{
    char buffer[ResponseBufferSize] = {};
    const RecordList additional = serviceRecords(service);

    for (int socket : sockets)
        mdns_announce_multicast(socket, buffer, sizeof(buffer), service.ptrRecord, nullptr, 0,
                                additional.records.data(), additional.count);
}

void sendGoodbye(const std::vector<int>& sockets, const ServiceData& service)
{
    char buffer[ResponseBufferSize] = {};
    const RecordList additional = serviceRecords(service);

    for (int socket : sockets)
        mdns_goodbye_multicast(socket, buffer, sizeof(buffer), service.ptrRecord, nullptr, 0,
                               additional.records.data(), additional.count);
}

void closeSockets(std::vector<int>& sockets)
{
    for (int socket : sockets)
        mdns_socket_close(socket);
    sockets.clear();
}

int serviceCallback(int socket, const sockaddr* from, size_t addressLength,
                    mdns_entry_type_t entry, uint16_t queryId, uint16_t queryType,
                    uint16_t queryClass, uint32_t /*ttl*/, const void* data,
                    size_t size, size_t nameOffset, size_t /*nameLength*/,
                    size_t /*recordOffset*/, size_t /*recordLength*/, void* userData)
{
    if (entry != MDNS_ENTRYTYPE_QUESTION)
        return 0;

    const auto* service = static_cast<const ServiceData*>(userData);
    const bool unicast = (queryClass & MDNS_UNICAST_RESPONSE) != 0;

    char nameBuffer[256] = {};
    size_t offset = nameOffset;
    const mdns_string_t questionName =
        mdns_string_extract(data, size, &offset, nameBuffer, sizeof(nameBuffer));

    if (sameName(questionName, DnsSdServiceType) &&
        acceptsRecordType(queryType, MDNS_RECORDTYPE_PTR)) {
        mdns_record_t answer{};
        answer.name = questionName;
        answer.type = MDNS_RECORDTYPE_PTR;
        answer.data.ptr.name = service->service;

        answerQuestion(socket, from, addressLength, queryId, queryType, unicast,
                       questionName, answer, {});
    } else if (sameName(questionName, service->service) &&
               acceptsRecordType(queryType, MDNS_RECORDTYPE_PTR)) {
        answerQuestion(socket, from, addressLength, queryId, queryType, unicast,
                       questionName, service->ptrRecord, serviceRecords(*service));
    } else if (sameName(questionName, service->serviceInstance) &&
               acceptsRecordType(queryType, MDNS_RECORDTYPE_SRV)) {
        answerQuestion(socket, from, addressLength, queryId, queryType, unicast,
                       questionName, service->srvRecord, instanceRecords(*service));
    } else if (sameName(questionName, service->hostnameQualified) &&
               hasIpv4Address(*service) &&
               acceptsRecordType(queryType, MDNS_RECORDTYPE_A)) {
        answerQuestion(socket, from, addressLength, queryId, queryType, unicast,
                       questionName, service->aRecord,
                       addressRecords(*service, MDNS_RECORDTYPE_A));
    } else if (sameName(questionName, service->hostnameQualified) &&
               hasIpv6Address(*service) &&
               acceptsRecordType(queryType, MDNS_RECORDTYPE_AAAA)) {
        answerQuestion(socket, from, addressLength, queryId, queryType, unicast,
                       questionName, service->aaaaRecord,
                       addressRecords(*service, MDNS_RECORDTYPE_AAAA));
    }

    return 0;
}

} // anonymous namespace


struct MdnsPublisher::Impl
{
    std::string serviceName;
    std::string serviceType;
    unsigned int port;

    std::string serviceTypeLocal;
    std::string serviceInstanceLocal;
    std::string hostnameLocal;

    ServiceData service;
    std::vector<int> sockets;

    std::atomic_bool running{false};
    std::thread listenThread;

    void listen()
    {
        char buffer[ResponseBufferSize] = {};

        while (running) {
            const auto readySockets = waitForReadableSockets(sockets, ResponsePollTimeout);
            for (int socket : readySockets)
                mdns_socket_listen(socket, buffer, sizeof(buffer), serviceCallback, &service);
        }
    }
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

    m_impl->serviceTypeLocal = m_impl->serviceType + ".local.";
    m_impl->serviceInstanceLocal = m_impl->serviceName + "." + m_impl->serviceType + ".local.";
    m_impl->hostnameLocal = getLocalHostname() + ".local.";

    m_impl->sockets = openMdnsServiceSockets();
    if (m_impl->sockets.empty()) {
        std::cerr << "MdnsPublisher: failed to open mDNS sockets\n";
        return;
    }

    m_impl->service = buildServiceData(m_impl->serviceTypeLocal, m_impl->serviceInstanceLocal,
                                       m_impl->hostnameLocal, getLocalAddresses(), m_impl->port);

    announceService(m_impl->sockets, m_impl->service);

    std::cout << "mDNS service published: " << m_impl->serviceInstanceLocal
              << " on port " << m_impl->port << "\n";

    m_impl->running = true;
    m_impl->listenThread = std::thread([this] { m_impl->listen(); });
}


void MdnsPublisher::stop()
{
    if (!m_impl->running)
        return;

    m_impl->running = false;
    if (m_impl->listenThread.joinable())
        m_impl->listenThread.join();

    sendGoodbye(m_impl->sockets, m_impl->service);
    closeSockets(m_impl->sockets);

    std::cout << "mDNS service unpublished\n";
}
