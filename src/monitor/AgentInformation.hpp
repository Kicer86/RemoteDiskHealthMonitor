
#pragma once

#include <QDebug>
#include <QHostAddress>
#include <QString>
#include <QtQml/qqmlregistration.h>

class AgentInformation
{
    Q_GADGET

public:
    enum class DetectionSource {
        ZeroConf,
        Hardcoded,
    };

    Q_ENUM(DetectionSource)

    AgentInformation(const QString& name, const QHostAddress& host, quint16 port, DetectionSource);

    const QString& name() const;
    const QHostAddress& host() const;
    quint16 port() const;
    DetectionSource detectionSource() const;

    bool operator==(const AgentInformation &) const;

private:
    friend uint qHash(const AgentInformation &, uint);

    QString m_name;
    QHostAddress m_host;
    quint16 m_port;
    DetectionSource m_detectionSource;
};


uint qHash(const AgentInformation &, uint seed);
QDebug operator<<(QDebug debug, const AgentInformation &);


namespace AgentDetection
{
    Q_NAMESPACE
    QML_NAMED_ELEMENT(AgentInformation)

    enum DetectionSource
    {
        ZeroConf  = static_cast<int>(AgentInformation::DetectionSource::ZeroConf),
        Hardcoded = static_cast<int>(AgentInformation::DetectionSource::Hardcoded),
    };

    Q_ENUM_NS(DetectionSource)
}
