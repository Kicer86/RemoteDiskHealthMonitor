
#include <cstdio>
#include <array>
#include <algorithm>
#include <cctype>
#include <iostream>

#include "../SmartReader.h"
#include "SmartCtlOutputParser.h"


namespace
{
    bool isValidDeviceId(const std::string& id)
    {
        return !id.empty() && id.size() <= 64 &&
               std::all_of(id.begin(), id.end(), [](char c) {
                   return std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_';
               });
    }

    std::string runSmartctl(const Disk& disk)
    {
        std::string output;

        if (!isValidDeviceId(disk.GetDeviceId()))
        {
            std::cerr << "Invalid device ID, skipping smartctl: " << disk.GetDeviceId() << '\n';
        }
        else
        {
            std::array<char, 4096> buffer;
            const std::string cmd = "smartctl -a /dev/" + disk.GetDeviceId();
            FILE* pipe = popen(cmd.c_str(), "r");
            if (pipe)
            {
                while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
                    output += buffer.data();
                pclose(pipe);
            }
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
