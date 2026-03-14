#pragma once

#include <string>

class Disk
{
public:
    Disk() = default;
    Disk(const std::string& _deviceId);
    Disk(const std::string& _deviceId, const std::string& _model);

    const std::string& GetDeviceId() const;
    const std::string& GetModel() const;
    const std::string& GetVendor() const;

    friend bool operator<(const Disk& lhs, const Disk& rhs)
    {
        return lhs.m_deviceId < rhs.m_deviceId;
    }

protected:
    std::string m_deviceId;
    std::string m_model;
    std::string m_vendor;

    static std::string detectVendor(const std::string& model);
};
