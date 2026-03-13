
#include <iostream>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "AgentsStatusProvider.hpp"
#include "common/JsonSerialize.h"


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

    if (it->sseReply)
    {
        it->sseReply->abort();
        it->sseReply->deleteLater();
    }

    m_connections.erase(it);
}


void AgentsStatusProvider::fetchInitialStatus(const AgentInformation& info)
{
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
        }
        else
        {
            std::cerr << "Failed to fetch status from " << info.name().toStdString()
                      << ": " << reply->errorString().toStdString() << "\n";
        }

        connectSse(info);
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

        processSseData(info);
    });

    QObject::connect(sseReply, &QNetworkReply::finished, this, [this, info, sseReply]() {
        sseReply->deleteLater();
        auto it = m_connections.find(info);
        if (it != m_connections.end())
        {
            it->sseReply = nullptr;
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
