#pragma once

#include <chrono>
#include <vector>

#include <nlohmann/json.hpp>

#include "common/GeneralHealth.h"
#include "agent/Disk.h"


struct RefreshPolicy
{
    std::chrono::seconds interval{0};
    bool proactiveCollection = false;
};


class IProbe
{
public:
    virtual ~IProbe() = default;

    virtual RefreshPolicy GetRefreshPolicy() const = 0;
    virtual void Refresh(const std::vector<Disk>& disks) = 0;

    virtual GeneralHealth::Health GetStatus(const Disk& _disk) = 0;
    virtual nlohmann::json GetRawData(const Disk& _disk) = 0;
};