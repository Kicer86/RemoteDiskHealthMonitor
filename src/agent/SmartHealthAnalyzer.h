#pragma once

#include "IProbe.h"
#include "ISmartReader.h"

#include <map>
#include <memory>
#include <string>

#include "common/SmartData.h"

class IVendorProfile;

class SmartHealthAnalyzer : public IProbe
{
public:
    explicit SmartHealthAnalyzer(std::unique_ptr<ISmartReader> reader);
    ~SmartHealthAnalyzer() override;

    RefreshPolicy GetRefreshPolicy() const override;
    void Refresh(const std::vector<Disk>& disks) override;

    GeneralHealth::Health GetStatus(const Disk& disk) override;
    nlohmann::json GetRawData(const Disk& disk) override;

private:
    std::unique_ptr<ISmartReader> m_reader;

    std::map<std::string, SmartData> m_cachedSmartData;
    std::map<std::string, SmartTestStatus> m_cachedTestStatus;

    static const IVendorProfile& profileFor(const std::string& vendor);
};
