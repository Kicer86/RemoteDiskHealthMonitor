
#include <cstdio>
#include <cstdlib>
#include <array>
#include <sstream>
#include <string>
#include <sys/wait.h>

#include "common/GeneralHealth.h"
#include "LinGeneralAnalyzer.h"
#include "DmesgParser.h"
#include "IPartitionsManager.h"

namespace
{
    struct CommandResult
    {
        std::string output;
        bool success = false;
    };

    std::string getCursorFilePath()
    {
        if (const char* runDir = std::getenv("XDG_RUNTIME_DIR"))
            return std::string(runDir) + "/rdhm-journal-cursor";

        return "/run/rdhm/journal-cursor";
    }

    std::string shellQuote(const std::string& value)
    {
        std::string quoted = "'";
        for (char c : value)
        {
            if (c == '\'')
                quoted += "'\\''";
            else
                quoted += c;
        }
        quoted += "'";
        return quoted;
    }

    bool exitStatusOk(int status)
    {
        return WIFEXITED(status) && WEXITSTATUS(status) == 0;
    }

    CommandResult runCommand(const std::string& cmd)
    {
        CommandResult result;
        std::array<char, 4096> buffer;

        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe)
            return result;

        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
            result.output += buffer.data();

        result.success = exitStatusOk(pclose(pipe));
        return result;
    }
}



LinGeneralAnalyzer::LinGeneralAnalyzer(std::shared_ptr<IPartitionsManager> manager)
    : m_partitionsManager(manager)
{
    m_useJournalctl = runCommand("command -v journalctl 2>/dev/null").success;
}


RefreshPolicy LinGeneralAnalyzer::GetRefreshPolicy() const
{
    return {std::chrono::hours(1), true};
}


GeneralHealth::Health LinGeneralAnalyzer::GetStatus(const Disk& disk) const
{
    return m_errors.contains(disk)
        ? GeneralHealth::Health::BAD
        : GeneralHealth::Health::GOOD;
}


nlohmann::json LinGeneralAnalyzer::GetRawData(const Disk& disk) const
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
    const auto cursorPath = getCursorFilePath();
    const auto journalCmd = "journalctl -k --cursor-file=" + shellQuote(cursorPath) +
                            " --no-pager -q --output=short 2>/dev/null";

    CommandResult commandResult;
    bool usedJournalctl = false;

    if (m_useJournalctl)
    {
        commandResult = runCommand(journalCmd);
        usedJournalctl = commandResult.success;
    }

    if (!usedJournalctl)
        commandResult = runCommand("dmesg 2>/dev/null");

    auto newErrors = DmesgParser::parse(commandResult.output, *m_partitionsManager);

    if (usedJournalctl)
    {
        // journalctl with cursor-file returns only new entries since last read.
        // Errors are accumulated permanently — once a disk reports an error it stays BAD
        // for the lifetime of the agent process. This is intentional: disk I/O errors
        // warrant investigation even if they stop recurring.
        for (auto& [disk, errors] : newErrors)
            m_errors[disk].merge(std::move(errors));
    }
    else
    {
        // dmesg reads the full ring buffer; replace entirely
        m_errors = std::move(newErrors);
    }
}
