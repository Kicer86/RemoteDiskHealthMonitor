#pragma once

#include "IProbe.h"
#include "ISmartReader.h"

#include <memory>

class IVendorProfile;

class SmartHealthAnalyzer : public IProbe
{
public:
    explicit SmartHealthAnalyzer(std::unique_ptr<ISmartReader> reader);
    ~SmartHealthAnalyzer() override;

    GeneralHealth::Health GetStatus(const Disk& disk) override;
    nlohmann::json GetRawData(const Disk& disk) override;

private:
    std::unique_ptr<ISmartReader> m_reader;

    static const IVendorProfile& profileFor(const std::string& vendor);
};
