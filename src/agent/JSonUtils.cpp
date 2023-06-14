
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
            QJsonObject json;
            json.insert("type", magic_enum::enum_name(dt));
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
            QJsonObject value;
            value.insert("raw_value", e.second.rawVal);
            value.insert("value", e.second.value);
            value.insert("word", e.second.worst);

            smartDataJSon.insert(SmartData::GetAttrTypeName(e.first), value);
        }

        return wrap(smartDataJSon, DataType::SmartTable);
    }
}
