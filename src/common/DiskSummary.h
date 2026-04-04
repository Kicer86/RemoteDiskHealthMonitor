#pragma once

#include <cstdint>
#include <optional>
#include <string>

struct SmartTestStatus
{
    bool running = false;
    int percentRemaining = 0;
    std::string lastResult;

    bool operator==(const SmartTestStatus&) const = default;
};

struct DiskSummary
{
    std::string model;
    std::string vendor;
    uint64_t capacityBytes = 0;
    std::string driveType;
    std::optional<int> temperatureC;
    std::optional<int64_t> powerOnHours;
    std::optional<SmartTestStatus> selfTestStatus;

    bool operator==(const DiskSummary&) const = default;
};
