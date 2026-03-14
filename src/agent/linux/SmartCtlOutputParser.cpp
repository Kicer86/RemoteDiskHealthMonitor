
#include <sstream>
#include <vector>

#include "agent/OutputParsersUtils.h"
#include "SmartCtlOutputParser.h"


namespace SmartCtlOutputParser
{

    namespace
    {
        // ─── ATA helpers ───

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

                if (rawAttributeSplitted.size() >= 10)
                {
                    const auto& id = rawAttributeSplitted[0];
                    const auto& name = rawAttributeSplitted[1];
                    const auto& value = rawAttributeSplitted[3];
                    const auto& worst = rawAttributeSplitted[4];
                    const auto& thresh = rawAttributeSplitted[5];
                    const auto& rawValue = rawAttributeSplitted[9];

                    try
                    {
                        const int threshVal = (thresh == "---") ? 0 : std::stoi(thresh);

                        smartData.attributes.push_back(SmartData::Attribute{
                            static_cast<uint8_t>(std::stoul(id)),
                            name,
                            std::stoi(value),
                            std::stoi(worst),
                            threshVal,
                            std::stoll(rawValue, nullptr, 0)
                        });
                    }
                    catch (const std::exception&) {}
                }
            }

            return smartData;
        }

        // ─── NVMe helpers ───

        bool isNvmeOutput(const std::vector<std::string>& lines)
        {
            for (const auto& line : lines)
                if (line.find("SMART/Health Information (NVMe") != std::string::npos)
                    return true;

            return false;
        }

        // Extract key-value lines from the NVMe health section.
        std::vector<std::pair<std::string, std::string>> nvmeHealthEntries(const std::vector<std::string>& lines)
        {
            std::vector<std::pair<std::string, std::string>> entries;

            auto it = lines.cbegin();
            for (; it != lines.cend(); ++it)
                if (it->find("SMART/Health Information (NVMe") != std::string::npos)
                    break;

            if (it != lines.cend())
                ++it;  // skip the section header

            for (; it != lines.cend() && !it->empty(); ++it)
            {
                const auto& line = *it;
                auto colonPos = line.find(':');
                if (colonPos == std::string::npos)
                    continue;

                auto key = simplified(line.substr(0, colonPos));
                auto val = simplified(line.substr(colonPos + 1));
                if (!key.empty())
                    entries.emplace_back(std::move(key), std::move(val));
            }

            return entries;
        }

        // Parse a numeric value from NVMe value string.
        // Handles: "100%", "50 Celsius", "0x00", "104 824 560 [53,6 TB]", plain integers.
        int64_t parseNvmeValue(const std::string& val)
        {
            // Remove percent sign
            std::string s = val;
            if (!s.empty() && s.back() == '%')
                s.pop_back();

            // Strip everything from '[' onward (e.g. "104 824 560 [53,6 TB]" → "104 824 560")
            auto bracketPos = s.find('[');
            if (bracketPos != std::string::npos)
                s = s.substr(0, bracketPos);

            // Strip text suffixes like " Celsius"
            // Keep only digits, spaces, minus, and "0x" hex prefix
            if (s.size() >= 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
                return std::stoll(s, nullptr, 16);

            // Remove spaces within numbers (locale-formatted: "104 824 560")
            std::string digits;
            bool seenDigit = false;
            bool negative = false;
            for (size_t i = 0; i < s.size(); ++i)
            {
                char c = s[i];
                if (c == '-' && !seenDigit)
                {
                    negative = true;
                    continue;
                }
                if (c >= '0' && c <= '9')
                {
                    digits += c;
                    seenDigit = true;
                }
                else if (c == ' ' && seenDigit)
                {
                    // Could be a locale separator or start of a suffix — peek ahead
                    if (i + 1 < s.size() && s[i + 1] >= '0' && s[i + 1] <= '9')
                        continue;  // space between digit groups
                    else
                        break;  // suffix like " Celsius"
                }
                else if (seenDigit)
                {
                    break;  // non-digit after digits
                }
            }

            if (digits.empty())
                return 0;

            int64_t result = std::stoll(digits);
            return negative ? -result : result;
        }

        SmartData parseNvmeHealth(const std::vector<std::string>& lines)
        {
            SmartData smartData;

            const auto entries = nvmeHealthEntries(lines);

            // Assign synthetic IDs for NVMe health fields.
            // Using a simple sequential scheme; these don't collide with ATA IDs
            // since they are purely internal identifiers.
            uint8_t nextId = 1;
            for (const auto& [key, val] : entries)
            {
                // Replace spaces with underscores in name for consistency with ATA naming
                std::string name;
                for (char c : key)
                    name += (c == ' ') ? '_' : c;

                int64_t rawVal = parseNvmeValue(val);

                // For Available_Spare / Available_Spare_Threshold, set value/threshold
                // so the existing health analyzer can use threshold-based checks.
                int value = 0;
                int threshold = 0;

                if (key == "Available Spare")
                    value = static_cast<int>(rawVal);
                else if (key == "Available Spare Threshold")
                    threshold = static_cast<int>(rawVal);

                smartData.attributes.push_back(SmartData::Attribute{
                    nextId++,
                    name,
                    value,
                    value,  // worst = value (no historical worst for NVMe)
                    threshold,
                    rawVal
                });
            }

            // Post-process: apply Available Spare Threshold to the Available Spare attribute
            int spareThreshold = 0;
            for (const auto& attr : smartData.attributes)
            {
                if (attr.name == "Available_Spare_Threshold")
                {
                    spareThreshold = static_cast<int>(attr.rawVal);
                    break;
                }
            }

            if (spareThreshold > 0)
            {
                for (auto& attr : smartData.attributes)
                {
                    if (attr.name == "Available_Spare")
                    {
                        attr.threshold = spareThreshold;
                        break;
                    }
                }
            }

            return smartData;
        }
    }

    SmartData parse(const std::string& smartCtlOutput)
    {
        const auto cleanLines = ParsersUtils::clean(smartCtlOutput);

        if (isNvmeOutput(cleanLines))
            return parseNvmeHealth(cleanLines);

        const auto attributeLines = smartAttributes(cleanLines);
        const auto table = parseRawTable(attributeLines);

        return table;
    }
}
