
#include <iostream>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "AgentsStatusProvider.hpp"
#include "common/JsonSerialize.h"
#include "common/constants.hpp"


AgentsStatusProvider::AgentsStatusProvider(QObject* parent)
    : IAgentsStatusProvider()
{
}


void AgentsStatusProvider::observe(const AgentInformation& info)
{
    if (m_connections.contains(info))
        return;

    m_connections.insert(info, AgentConnection{});

    // Fetch initial status, then trigger refresh, then connect SSE
    fetchInitialStatus(info);
}


void AgentsStatusProvider::unobserve(const AgentInformation& info)
{
    auto it = m_connections.find(info);
    if (it == m_connections.end())
        return;

    QNetworkReply* reply = it->sseReply;
    it->sseReply = nullptr;
    m_connections.erase(it);

    if (reply)
    {
        reply->disconnect(this);
        reply->abort();
        reply->deleteLater();
    }
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


void AgentsStatusProvider::fetchAgentInfo(const AgentInformation& info)
{
    const QUrl url = QStringLiteral("http://%1:%2/api/v1/info")
                         .arg(info.host().toString())
                         .arg(info.port());

    QNetworkRequest req(url);
    QNetworkReply* reply = m_nam.get(req);

    QObject::connect(reply, &QNetworkReply::finished, this, [this, info, reply]() {
        reply->deleteLater();

        if (reply->error() == QNetworkReply::NoError)
        {
            try
            {
                nlohmann::json j = nlohmann::json::parse(reply->readAll().toStdString());
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
        }
    });
}


void AgentsStatusProvider::connectSse(const AgentInformation& info)
{
    auto it = m_connections.find(info);
    if (it == m_connections.end())
        return;

    const QUrl url = QStringLiteral("http://%1:%2/api/v1/events")
                         .arg(info.host().toString())
                         .arg(info.port());

    QNetworkRequest req(url);
    req.setRawHeader("Accept", "text/event-stream");
    req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);

    QNetworkReply* sseReply = m_nam.get(req);
    it->sseReply = sseReply;
    it->sseBuffer.clear();

    QObject::connect(sseReply, &QNetworkReply::readyRead, this, [this, info]() {
        auto it = m_connections.find(info);
        if (it != m_connections.end())
            it->reconnectDelayMs = 1000;   // reset backoff on successful data

        emit connectionStateChanged(info, ConnectionState::Connected);
        processSseData(info);
    });

    QObject::connect(sseReply, &QNetworkReply::finished, this, [this, info, sseReply]() {
        sseReply->deleteLater();
        auto it = m_connections.find(info);
        if (it != m_connections.end())
        {
            it->sseReply = nullptr;
            emit connectionStateChanged(info, ConnectionState::Disconnected);
            scheduleSseReconnect(info);
        }
    });
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


void AgentsStatusProvider::processSseData(const AgentInformation& info)
{
    auto it = m_connections.find(info);
    if (it == m_connections.end() || !it->sseReply)
        return;

    it->sseBuffer.append(it->sseReply->readAll());

    // Parse SSE messages: lines separated by \n\n
    while (true)
    {
        int idx = it->sseBuffer.indexOf("\n\n");
        if (idx < 0)
            break;

        QByteArray message = it->sseBuffer.left(idx);
        it->sseBuffer.remove(0, idx + 2);

        // Parse SSE fields
        QByteArray data;
        for (const QByteArray& line : message.split('\n'))
        {
            if (line.startsWith("data: "))
                data.append(line.mid(6));
        }

        if (!data.isEmpty())
            parseStatusJson(info, data);
    }
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
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON parse error for agent " << info.name().toStdString()
                  << ": " << e.what() << "\n";
    }
}
