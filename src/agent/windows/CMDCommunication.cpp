#include "CMDCommunication.h"
#include <array>
#include <cstdio>
#include <iostream>
#include <algorithm>

namespace
{
    std::string runCommand(const std::string& cmd)
    {
        std::string output;
        std::array<char, 4096> buffer;

        FILE* pipe = _popen(cmd.c_str(), "r");
        if (pipe)
        {
            while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
                output += buffer.data();
            _pclose(pipe);
        }

        return output;
    }
}

GeneralHealth::Health CMDCommunication::CollectDiskStatus(const Disk& _disk)
{
    const std::string commandResult = ExecuteDiscStatusCommand(_disk);

    if (commandResult == "OK")
        return GeneralHealth::GOOD;
    else if (commandResult == "Degraded")
        return GeneralHealth::CHECK_STATUS;
    else if (commandResult == "PredFail")
        return GeneralHealth::BAD;

    return GeneralHealth::UNKNOWN;
}

bool CMDCommunication::CompareDeviceIdWithInstanceName(const Disk& _disk, std::string _instanceName)
{
    std::string diskInstanceName = GetInstanceName(_disk);

    auto diskPos = _instanceName.find("_0");
    _instanceName = _instanceName.substr(0, diskPos);

    ChangeStringToLowerCase(_instanceName);
    ChangeStringToLowerCase(diskInstanceName);

    return ( diskInstanceName.compare(_instanceName) == 0 );
}

std::string CMDCommunication::ExecuteDiscStatusCommand(const Disk& _disk) const
{
    std::string ret = runCommand("wmic diskdrive get deviceid,status");

    auto diskPos = ret.find(_disk.GetDeviceId());
    if (diskPos == std::string::npos)
        return {};

    auto statusPosStart = ret.find_first_not_of(' ', diskPos + (_disk.GetDeviceId()).size());
    auto statusPosStop = ret.find_first_of(" \r\n", statusPosStart);
    ret = ret.substr(statusPosStart, statusPosStop - statusPosStart);
    return ret;
}

std::string CMDCommunication::GetInstanceName(const Disk& _disk) const
{
    std::string ret = runCommand("wmic diskdrive get DeviceID,PNPDeviceID");

    auto diskPos = ret.find(_disk.GetDeviceId());
    if (diskPos == std::string::npos)
        return {};

    auto statusPosStart = ret.find_first_not_of(' ', diskPos + (_disk.GetDeviceId()).size());
    auto statusPosStop = ret.find_first_of(" \r\n", statusPosStart);
    ret = ret.substr(statusPosStart, statusPosStop - statusPosStart);
    return ret;
}

void CMDCommunication::ChangeStringToLowerCase(std::string& _string) const
{
    std::transform(_string.begin(), _string.end(), _string.begin(), ::tolower);
}
