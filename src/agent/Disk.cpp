#include "Disk.h"

#include <algorithm>

Disk::Disk(const std::string& _deviceId)
    : m_deviceId(_deviceId)
{
}

Disk::Disk(const std::string& _deviceId, const std::string& _model)
    : m_deviceId(_deviceId)
    , m_model(_model)
    , m_vendor(detectVendor(_model))
{
}

Disk::Disk(const std::string& _deviceId, const std::string& _model,
           uint64_t capacity, const std::string& driveType)
    : m_deviceId(_deviceId)
    , m_model(_model)
    , m_vendor(detectVendor(_model))
    , m_capacity(capacity)
    , m_driveType(driveType)
{
}

const std::string& Disk::GetDeviceId() const
{
    return m_deviceId;
}

const std::string& Disk::GetModel() const
{
    return m_model;
}

const std::string& Disk::GetVendor() const
{
    return m_vendor;
}

uint64_t Disk::GetCapacity() const
{
    return m_capacity;
}

const std::string& Disk::GetDriveType() const
{
    return m_driveType;
}

std::string Disk::detectVendor(const std::string& model)
{
    std::string lower = model;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    if (lower.find("samsung") != std::string::npos)
        return "Samsung";
    if (lower.find("wdc ") != std::string::npos || lower.find("western digital") != std::string::npos)
        return "WDC";
    if (lower.substr(0, 2) == "st" || lower.find("seagate") != std::string::npos)
        return "Seagate";
    if (lower.find("toshiba") != std::string::npos || lower.substr(0, 4) == "hdwd" || lower.substr(0, 2) == "dt")
        return "Toshiba";
    if (lower.find("hitachi") != std::string::npos || lower.find("hgst") != std::string::npos)
        return "HGST";
    if (lower.find("intel") != std::string::npos)
        return "Intel";
    if (lower.find("crucial") != std::string::npos || lower.find("micron") != std::string::npos)
        return "Micron";
    if (lower.find("kingston") != std::string::npos)
        return "Kingston";
    if (lower.find("sandisk") != std::string::npos)
        return "SanDisk";

    return {};
}
