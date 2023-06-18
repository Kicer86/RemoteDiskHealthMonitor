
#include <QStringList>

#include "common/OutputParsersUtils.h"
#include "SmartCtlOutputParser.h"


namespace SmartCtlOutputParser
{

    namespace
    {
        QStringList smartAttributes(const QStringList& output)
        {
            QStringList attributes;

            auto it = output.constBegin();

            // skip all lines before attributes table
            for(; it != output.constEnd() && *it != "Vendor Specific SMART Attributes with Thresholds:"; ++it);

            // skip table header
            if (it != output.constEnd())
                ++it;

            if (it != output.constEnd())
                ++it;

            // copy table
            for(; it != output.constEnd() && *it != ""; ++it)
                attributes.append(*it);

            return attributes;
        }

        SmartData parseRawTable(const QStringList& table)
        {
            SmartData smartData;

            for(const QString& rawAttribute: table)
            {
                const QString rawAttributeSimplified = rawAttribute.simplified();   // drop all redundant spaces
                const QStringList rawAttributeSplitted = rawAttributeSimplified.split(' ');

                if (rawAttributeSplitted.size() >= 10)  // at least 10 columns expected (ID# ATTRIBUTE_NAME FLAG VALUE WORST THRESH TYPE UPDATED WHEN_FAILED RAW_VALUE (extra details))
                {
                    const QString& id = rawAttributeSplitted[0];        // ID#
                    const QString& value = rawAttributeSplitted[3];     // VALUE
                    const QString& worst = rawAttributeSplitted[4];     // WORST
                    const QString& rawValue = rawAttributeSplitted[9];  // RAW_VALUE

                    smartData.add(
                        static_cast<SmartData::SmartAttribute>(id.toUInt()),
                        SmartData::AttrData {
                            value.toInt(),
                            worst.toInt(),
                            rawValue.toInt()
                        }
                    );
                }
            }

            return smartData;
        }
    }

    SmartData parse(const QByteArray& smartCtlOutput)
    {
        const auto cleanLines = ParsersUtils::clean(smartCtlOutput);
        const auto attributeLines = smartAttributes(cleanLines);
        const auto table = parseRawTable(attributeLines);

        return table;
    }
}
