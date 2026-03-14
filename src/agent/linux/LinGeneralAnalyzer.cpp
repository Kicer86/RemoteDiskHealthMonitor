
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
    refreshState();
}


GeneralHealth::Health LinGeneralAnalyzer::GetStatus(const Disk& disk)
{
    return m_errors.find(disk) == m_errors.end()?
        GeneralHealth::Health::GOOD :
        GeneralHealth::Health::BAD;
}


IProbe::RawData LinGeneralAnalyzer::GetRawData(const Disk& disk)
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

    return result;
}


void LinGeneralAnalyzer::refreshState()
{
    std::string output;
    std::array<char, 4096> buffer;

    FILE* pipe = popen("dmesg", "r");
    if (pipe)
    {
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
            output += buffer.data();
        pclose(pipe);
    }

    m_errors = DmesgParser::parse(output, *m_partitionsManager);
}
