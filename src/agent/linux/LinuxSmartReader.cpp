
#include <cstdio>
#include <array>
#include <iostream>

#include "../SmartReader.h"
#include "SmartCtlOutputParser.h"


namespace
{
    std::string runSmartctl(const Disk& disk)
    {
        std::string output;
        std::array<char, 4096> buffer;

        const std::string cmd = "smartctl -a /dev/" + disk.GetDeviceId();
        FILE* pipe = popen(cmd.c_str(), "r");
        if (pipe)
        {
            while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
                output += buffer.data();
            pclose(pipe);
        }

        return output;
    }
}


SmartData SmartReader::ReadSMARTData(const Disk& disk)
{
    return SmartCtlOutputParser::parse(runSmartctl(disk));
}


GeneralHealth::Health SmartReader::ReadDisksStatus(const Disk &)
{
    return GeneralHealth::GOOD;
}

std::string SmartReader::ReadRawOutput(const Disk& disk)
{
    return runSmartctl(disk);
}

SmartTestStatus SmartReader::ReadTestStatus(const Disk& disk)
{
    return SmartCtlOutputParser::parseTestStatus(runSmartctl(disk));
}
