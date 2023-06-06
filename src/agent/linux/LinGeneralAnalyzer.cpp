
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>

#include "common/GeneralHealth.h"
#include "LinGeneralAnalyzer.h"
#include "DmesgParser.h"
#include "IPartitionsManager.h"


LinGeneralAnalyzer::LinGeneralAnalyzer(std::shared_ptr<IPartitionsManager> manager)
    : m_partitionsManager(manager)
{
    refreshState();
}


GeneralHealth::Health LinGeneralAnalyzer::GetStatus(const Disk& disk)
{
    return m_errors.find(disk) == m_errors.end()?
        GeneralHealth::Health::GOOD :
        GeneralHealth::Health::BAD;
}


QString LinGeneralAnalyzer::GetJSonData(const Disk& disk)
{
    QJsonObject jsonObject;

    auto it = m_errors.find(disk);
    if (it != m_errors.end())
    {
        QJsonArray errors;
        std::copy(it->second.begin(), it->second.end(), std::back_inserter(errors));

        jsonObject.insert("errors", errors);
    }

    QJsonDocument doc(jsonObject);
    return doc.toJson(QJsonDocument::Compact);
}


void LinGeneralAnalyzer::refreshState()
{
    QProcess dmesg;

    dmesg.start("dmesg", {}, QProcess::ReadOnly);
    dmesg.waitForFinished(5000);
    const QByteArray output = dmesg.readAll();

    m_errors = DmesgParser::parse(output, *m_partitionsManager);
}
