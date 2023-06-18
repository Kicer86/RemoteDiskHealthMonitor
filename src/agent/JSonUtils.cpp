
#include <QJsonDocument>
#include <QJsonObject>

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
}
