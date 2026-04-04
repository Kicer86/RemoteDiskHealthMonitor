#pragma once

#include <nlohmann/json.hpp>

#include "GeneralHealth.h"
#include "SmartData.h"
#include "ProbeStatus.h"
#include "DiskSummary.h"
#include "DiskInfo.h"


// GeneralHealth::Health
inline void to_json(nlohmann::json& j, const GeneralHealth::Health& h)
{
    j = healthToString(h);
}

inline void from_json(const nlohmann::json& j, GeneralHealth::Health& h)
{
    h = healthFromString(j.get<std::string>());
}


// SmartData::Attribute
inline void to_json(nlohmann::json& j, const SmartData::Attribute& a)
{
    j = nlohmann::json{
        {"id", a.id}, {"name", a.name},
        {"value", a.value}, {"worst", a.worst},
        {"threshold", a.threshold}, {"rawVal", a.rawVal}
    };
}

inline void from_json(const nlohmann::json& j, SmartData::Attribute& a)
{
    j.at("id").get_to(a.id);
    j.at("name").get_to(a.name);
    j.at("value").get_to(a.value);
    j.at("worst").get_to(a.worst);
    j.at("threshold").get_to(a.threshold);
    j.at("rawVal").get_to(a.rawVal);
}


// SmartData
inline void to_json(nlohmann::json& j, const SmartData& s)
{
    j = nlohmann::json{{"attributes", s.attributes}};
}

inline void from_json(const nlohmann::json& j, SmartData& s)
{
    j.at("attributes").get_to(s.attributes);
}


// SmartTestStatus
inline void to_json(nlohmann::json& j, const SmartTestStatus& s)
{
    j = nlohmann::json{
        {"running", s.running},
        {"percentRemaining", s.percentRemaining},
        {"lastResult", s.lastResult}
    };
}

inline void from_json(const nlohmann::json& j, SmartTestStatus& s)
{
    j.at("running").get_to(s.running);
    j.at("percentRemaining").get_to(s.percentRemaining);
    j.at("lastResult").get_to(s.lastResult);
}


// DiskSummary
inline void to_json(nlohmann::json& j, const DiskSummary& s)
{
    j = nlohmann::json{
        {"model", s.model},
        {"vendor", s.vendor},
        {"capacityBytes", s.capacityBytes},
        {"driveType", s.driveType}
    };
    if (s.temperatureC)
        j["temperatureC"] = *s.temperatureC;
    if (s.powerOnHours)
        j["powerOnHours"] = *s.powerOnHours;
    if (s.selfTestStatus)
        j["selfTestStatus"] = *s.selfTestStatus;
}

inline void from_json(const nlohmann::json& j, DiskSummary& s)
{
    j.at("model").get_to(s.model);
    j.at("vendor").get_to(s.vendor);
    j.at("capacityBytes").get_to(s.capacityBytes);
    j.at("driveType").get_to(s.driveType);
    if (j.contains("temperatureC"))
        s.temperatureC = j.at("temperatureC").get<int>();
    if (j.contains("powerOnHours"))
        s.powerOnHours = j.at("powerOnHours").get<int64_t>();
    if (j.contains("selfTestStatus"))
        s.selfTestStatus = j.at("selfTestStatus").get<SmartTestStatus>();
}


// ProbeStatus
inline void to_json(nlohmann::json& j, const ProbeStatus& p)
{
    j = nlohmann::json{{"health", p.health}, {"rawData", p.rawData}};
}

inline void from_json(const nlohmann::json& j, ProbeStatus& p)
{
    j.at("health").get_to(p.health);
    p.rawData = j.at("rawData");
}


// DiskInfo
inline void to_json(nlohmann::json& j, const DiskInfo& d)
{
    j = nlohmann::json{
        {"name", d.GetName()},
        {"health", d.GetHealth()},
        {"probes", d.GetProbesStatuses()},
        {"summary", d.GetSummary()}
    };
}

inline void from_json(const nlohmann::json& j, DiskInfo& d)
{
    d.SetName(j.at("name").get<std::string>());
    d.SetHealth(j.at("health").get<GeneralHealth::Health>());
    d.SetProbesStatuses(j.at("probes").get<std::vector<ProbeStatus>>());
    if (j.contains("summary"))
        d.SetSummary(j.at("summary").get<DiskSummary>());
}
