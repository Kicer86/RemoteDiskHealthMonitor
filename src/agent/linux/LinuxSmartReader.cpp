
#include <cstdio>
#include <array>
#include <iostream>

#include "../SmartReader.h"
#include "SmartCtlOutputParser.h"


SmartData SmartReader::ReadSMARTData(const Disk& disk)
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

    const auto entries = SmartCtlOutputParser::parse(output);

    return entries;
}


GeneralHealth SmartReader::ReadDisksStatus(const Disk &)
{
    return GeneralHealth::GOOD;
}
