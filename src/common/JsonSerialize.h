#pragma once

#include <nlohmann/json.hpp>

#include "GeneralHealth.h"
#include "SmartData.h"
#include "ProbeStatus.h"
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
        {"probes", d.GetProbesStatuses()}
    };
}

inline void from_json(const nlohmann::json& j, DiskInfo& d)
{
    d.SetName(j.at("name").get<std::string>());
    d.SetHealth(j.at("health").get<GeneralHealth::Health>());
    d.SetProbesStatuses(j.at("probes").get<std::vector<ProbeStatus>>());
}
