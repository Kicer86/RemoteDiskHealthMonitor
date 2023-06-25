
#include <QJsonDocument>

#include <magic_enum.hpp>

#include "JSonUtils.hpp"


namespace JSonUtils
{
    enum class DataType
    {
        SmartTable,
    };

    namespace
    {
        QString wrap(const QJsonObject& data, DataType dt)
        {
            const std::string dataType(magic_enum::enum_name(dt));
            QJsonObject json;
            json.insert("type", QString::fromStdString(dataType));
            json.insert("data", data);

            QJsonDocument doc(json);
            return doc.toJson(QJsonDocument::Compact);
        }


        QJsonObject ProbeStatusToJson(const ProbeStatus& probe)
        {
            QJsonObject probeJson;

            probeJson["status"] = probe.health;
            probeJson["details"] = probe.jsonData;

            return probeJson;
        }


        QJsonArray ProbeStatusToJson(const std::vector<ProbeStatus>& probes)
        {
            QJsonArray probesJson;

            for(const auto& probe: probes)
            {
                const auto probeJson = ProbeStatusToJson(probe);
                probesJson.append(probeJson);
            }

            return probesJson;
        }


        QJsonObject DiskInfoToJSon(const DiskInfo& di)
        {
            QJsonObject diskJson;

            diskJson["name"] = di.GetName();
            diskJson["status"] = di.GetHealth();
            diskJson["probes"] = ProbeStatusToJson(di.GetProbesStatuses());

            return diskJson;
        }

    }


    QString SmartDataToJSon(const SmartData& data)
    {
        QJsonObject smartDataJSon;

        for (const auto& e: data.data())
        {
            const auto name = SmartData::GetAttrTypeName(e.first);
            if (name)
            {
                QJsonObject value;
                value.insert("raw_value", e.second.rawVal);
                value.insert("value", e.second.value);
                value.insert("worst", e.second.worst);

                smartDataJSon.insert(name.value(), value);
            }
        }

        return wrap(smartDataJSon, DataType::SmartTable);
    }


    QJsonArray DiskInfoToJSon(const std::vector<DiskInfo>& di)
    {
        QJsonArray disksJson;

        for(const auto& disk: di)
        {
            const auto diskJson = DiskInfoToJSon(disk);
            disksJson.append(diskJson);
        }

        return disksJson;
    }

}
