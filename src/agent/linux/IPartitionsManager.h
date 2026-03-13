
#pragma once

#include <string>

class IPartitionsManager
{
public:
    virtual ~IPartitionsManager() = default;

    virtual bool isPartition(const std::string& deviceName) const = 0;
    virtual std::string diskForPartition(const std::string& deviceName) const = 0;
};
