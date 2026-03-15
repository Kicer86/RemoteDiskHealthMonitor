
#include <cstdio>
#include <array>
#include <sstream>

#include "common/GeneralHealth.h"
#include "LinGeneralAnalyzer.h"
#include "DmesgParser.h"
#include "IPartitionsManager.h"


LinGeneralAnalyzer::LinGeneralAnalyzer(std::shared_ptr<IPartitionsManager> manager)
    : m_partitionsManager(manager)
{
    FILE* pipe = popen("which journalctl", "r");
    if (pipe)
    {
        char buf[256];
        m_useJournalctl = (fgets(buf, sizeof(buf), pipe) != nullptr);
        pclose(pipe);
    }
}


RefreshPolicy LinGeneralAnalyzer::GetRefreshPolicy() const
{
    return {std::chrono::hours(1), true};
}


GeneralHealth::Health LinGeneralAnalyzer::GetStatus(const Disk& disk)
{
    return m_errors.find(disk) == m_errors.end()?
        GeneralHealth::Health::GOOD :
        GeneralHealth::Health::BAD;
}


nlohmann::json LinGeneralAnalyzer::GetRawData(const Disk& disk)
{
    std::string result;

    auto it = m_errors.find(disk);

    if (it != m_errors.end())
    {
        std::ostringstream oss;
        bool first = true;
        for (const auto& err : it->second)
        {
            if (!first) oss << '\n';
            oss << err;
            first = false;
        }
        result = oss.str();
    }

    return nlohmann::json{{"type", "text"}, {"value", result}};
}


void LinGeneralAnalyzer::Refresh(const std::vector<Disk>&)
{
    std::string output;
    std::array<char, 4096> buffer;

    const char* cmd = m_useJournalctl
        ? "journalctl -k --cursor-file=/tmp/rdhm-journal-cursor --no-pager -q --output=short"
        : "dmesg";

    FILE* pipe = popen(cmd, "r");
    if (pipe)
    {
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
            output += buffer.data();
        pclose(pipe);
    }

    auto newErrors = DmesgParser::parse(output, *m_partitionsManager);

    if (m_useJournalctl)
    {
        // journalctl with cursor-file returns only new entries;
        // accumulate into existing errors
        for (auto& [disk, errors] : newErrors)
            m_errors[disk].merge(std::move(errors));
    }
    else
    {
        // dmesg reads the full ring buffer; replace entirely
        m_errors = std::move(newErrors);
    }
}
