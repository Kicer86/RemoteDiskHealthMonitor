
#include <sstream>
#include <vector>

#include "common/OutputParsersUtils.h"
#include "SmartCtlOutputParser.h"


namespace SmartCtlOutputParser
{

    namespace
    {
        std::vector<std::string> smartAttributes(const std::vector<std::string>& output)
        {
            std::vector<std::string> attributes;

            auto it = output.cbegin();

            // skip all lines before attributes table
            for(; it != output.cend() && *it != "Vendor Specific SMART Attributes with Thresholds:"; ++it);

            // skip table header
            if (it != output.cend())
                ++it;

            if (it != output.cend())
                ++it;

            // copy table
            for(; it != output.cend() && !it->empty(); ++it)
                attributes.push_back(*it);

            return attributes;
        }

        // Simplify whitespace: collapse multiple spaces/tabs into single space, trim
        std::string simplified(const std::string& str)
        {
            std::string result;
            bool lastWasSpace = true;
            for (char c : str)
            {
                if (c == ' ' || c == '\t')
                {
                    if (!lastWasSpace)
                    {
                        result += ' ';
                        lastWasSpace = true;
                    }
                }
                else
                {
                    result += c;
                    lastWasSpace = false;
                }
            }
            // trim trailing space
            if (!result.empty() && result.back() == ' ')
                result.pop_back();
            // trim leading space
            if (!result.empty() && result.front() == ' ')
                result.erase(result.begin());
            return result;
        }

        std::vector<std::string> splitBySpace(const std::string& str)
        {
            std::vector<std::string> parts;
            std::istringstream stream(str);
            std::string part;
            while (stream >> part)
                parts.push_back(part);
            return parts;
        }

        SmartData parseRawTable(const std::vector<std::string>& table)
        {
            SmartData smartData;

            for(const auto& rawAttribute: table)
            {
                const auto rawAttributeSplitted = splitBySpace(simplified(rawAttribute));

                if (rawAttributeSplitted.size() == 10)
                {
                    const auto& id = rawAttributeSplitted[0];
                    const auto& value = rawAttributeSplitted[3];
                    const auto& worst = rawAttributeSplitted[4];
                    const auto& rawValue = rawAttributeSplitted[9];

                    smartData.smartData.emplace(
                        static_cast<SmartData::SmartAttribute>(std::stoul(id)),
                        SmartData::AttrData {
                            std::stoi(value),
                            std::stoi(worst),
                            std::stoi(rawValue)
                        }
                    );
                }
            }

            return smartData;
        }
    }

    SmartData parse(const std::string& smartCtlOutput)
    {
        const auto cleanLines = ParsersUtils::clean(smartCtlOutput);
        const auto attributeLines = smartAttributes(cleanLines);
        const auto table = parseRawTable(attributeLines);

        return table;
    }
}
