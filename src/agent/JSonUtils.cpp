
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


        ProbeStatus JSonToProbeStatus(const QJsonObject& probeJSon)
        {
            ProbeStatus probeStatus{
                .health = static_cast<GeneralHealth::Health>(probeJSon["status"].toInt()),
                .jsonData = probeJSon["details"].toString()
            };

            return probeStatus;
        }


        QJsonArray ProbeStatusToJson(const std::vector<ProbeStatus>& probes)
        {
            QJsonArray probesJson;

            for (const auto& probe: probes)
            {
                const auto probeJson = ProbeStatusToJson(probe);
                probesJson.append(probeJson);
            }

            return probesJson;
        }


        std::vector<ProbeStatus> JSonToProbesStatus(const QJsonArray& probesJSon)
        {
            std::vector<ProbeStatus> probes;

            for (const auto& probeJSon: probesJSon)
            {
                const auto probe = JSonToProbeStatus(probeJSon.toObject());
                probes.push_back(probe);
            }

            return probes;
        }


        QJsonObject DiskInfoToJSon(const DiskInfo& di)
        {
            QJsonObject diskJson;

            diskJson["name"] = di.GetName();
            diskJson["status"] = di.GetHealth();
            diskJson["probes"] = ProbeStatusToJson(di.GetProbesStatuses());

            return diskJson;
        }

        DiskInfo JSonToDisk(const QJsonObject& jsonDisk)
        {
            const auto probesJson = jsonDisk["probes"].toArray();
            const auto name = jsonDisk["name"].toString();
            const auto status = jsonDisk["status"].toInt();

            const auto probes = JSonToProbesStatus(probesJson);

            const DiskInfo di(name, static_cast<GeneralHealth::Health>(status), probes);

            return di;
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


    std::vector<DiskInfo> JSonToDiskInfo(const QJsonArray& disksJson)
    {
        std::vector<DiskInfo> disks;

        for(const auto& diskJson: disksJson)
        {
            const auto disk = JSonToDisk(diskJson.toObject());
            disks.push_back(disk);
        }

        return disks;
    }

}