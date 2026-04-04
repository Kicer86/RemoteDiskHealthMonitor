
#include "OutputParsersUtils.h"
#include <sstream>
#include <algorithm>

namespace ParsersUtils
{

    namespace
    {
        std::string trim(const std::string& str)
        {
            const auto start = str.find_first_not_of(" \t\r\n");
            if (start == std::string::npos) return "";
            const auto end = str.find_last_not_of(" \t\r\n");
            return str.substr(start, end - start + 1);
        }

        std::vector<std::string> cleanup(const std::vector<std::string>& list)
        {
            std::vector<std::string> clean;
            clean.reserve(list.size());

            for (const auto& entry : list)
                clean.push_back(trim(entry));

            return clean;
        }

        std::vector<std::string> trimList(std::vector<std::string> list)
        {
            while (!list.empty() && list.front().empty())
                list.erase(list.begin());

            while (!list.empty() && list.back().empty())
                list.pop_back();

            return list;
        }
    }


    std::vector<std::string> clean(const std::string& input)
    {
        std::vector<std::string> lines;
        std::istringstream stream(input);
        std::string line;

        while (std::getline(stream, line))
            lines.push_back(line);

        const auto cleanLines = cleanup(lines);
        const auto trimmed = trimList(cleanLines);

        return trimmed;
    }
}
