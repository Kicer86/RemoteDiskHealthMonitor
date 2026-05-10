#include "CMDCommunication.h"
#include <array>
#include <cctype>
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

    std::string tokenAfterDeviceId(const std::string& output, const std::string& deviceId)
    {
        auto diskPos = output.find(deviceId);
        if (diskPos == std::string::npos)
            return {};

        auto valueStart = output.find_first_not_of(" \t\r\n", diskPos + deviceId.size());
        if (valueStart == std::string::npos)
            return {};

        auto valueStop = output.find_first_of(" \t\r\n", valueStart);
        return output.substr(valueStart, valueStop == std::string::npos
            ? std::string::npos
            : valueStop - valueStart);
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
    if (diskInstanceName.empty() || _instanceName.empty())
        return false;

    auto diskPos = _instanceName.find("_0");
    _instanceName = _instanceName.substr(0, diskPos);

    ChangeStringToLowerCase(_instanceName);
    ChangeStringToLowerCase(diskInstanceName);

    return ( diskInstanceName.compare(_instanceName) == 0 );
}

std::string CMDCommunication::ExecuteDiscStatusCommand(const Disk& _disk) const
{
    const std::string ret = runCommand("wmic diskdrive get deviceid,status");
    return tokenAfterDeviceId(ret, _disk.GetDeviceId());
}

std::string CMDCommunication::GetInstanceName(const Disk& _disk) const
{
    const std::string ret = runCommand("wmic diskdrive get DeviceID,PNPDeviceID");
    return tokenAfterDeviceId(ret, _disk.GetDeviceId());
}

void CMDCommunication::ChangeStringToLowerCase(std::string& _string) const
{
    std::transform(_string.begin(), _string.end(), _string.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
}
