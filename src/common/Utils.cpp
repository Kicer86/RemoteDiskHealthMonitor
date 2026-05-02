
#include <string>
#include <array>
#include <iomanip>
#include <sstream>

#include "Utils.h"


std::string formatBytes(std::uint64_t bytes)
{
    static const std::array<const char*, 7> units =
    {
        "B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB"
    };

    double size = static_cast<double>(bytes);
    std::size_t unitIndex = 0;

    while(size >= 1024.0 && unitIndex < units.size() - 1)
    {
        size /= 1024.0;
        ++unitIndex;
    }

    std::ostringstream oss;

    if(unitIndex == 0)
        oss << static_cast<std::uint64_t>(size) << " " << units[unitIndex];
    else
        oss << std::fixed << std::setprecision(2) << size << " " << units[unitIndex];

    return oss.str();
}


std::string formatTable(const std::span<std::vector<std::string>> rows)
{
    if(rows.empty())
        return {};

    // liczba kolumn = max długość wiersza
    std::size_t cols = 0;
    for(const auto& r : rows)
        cols = std::max(cols, r.size());

    // szerokości kolumn
    std::vector<std::size_t> widths(cols, 0);
    for(const auto& r : rows)
    {
        for(std::size_t c = 0; c < r.size(); ++c)
            widths[c] = std::max(widths[c], r[c].size());
    }

    std::ostringstream oss;

    for(const auto& r : rows)
    {
        for(std::size_t c = 0; c < cols; ++c)
        {
            const std::string& cell = (c < r.size()) ? r[c] : "";

            oss << cell;

            // padding (1 spacja odstępu między kolumnami)
            std::size_t pad = widths[c] - cell.size();
            for(std::size_t i = 0; i < pad + 1; ++i)
                oss << ' ';
        }
        oss << '\n';
    }

    return oss.str();
}
