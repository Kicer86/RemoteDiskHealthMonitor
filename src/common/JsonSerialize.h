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


// SmartData::AttrData
inline void to_json(nlohmann::json& j, const SmartData::AttrData& a)
{
    j = nlohmann::json{{"value", a.value}, {"worst", a.worst}, {"rawVal", a.rawVal}};
}

inline void from_json(const nlohmann::json& j, SmartData::AttrData& a)
{
    j.at("value").get_to(a.value);
    j.at("worst").get_to(a.worst);
    j.at("rawVal").get_to(a.rawVal);
}


// SmartData
inline void to_json(nlohmann::json& j, const SmartData& s)
{
    nlohmann::json attrs = nlohmann::json::object();
    for (const auto& [attr, data] : s.smartData)
        attrs[SmartData::GetAttrTypeName(attr)] = data;

    j = nlohmann::json{{"attributes", attrs}};
}

inline void from_json(const nlohmann::json& j, SmartData& s)
{
    s.smartData.clear();
    const auto& attrs = j.at("attributes");

    // Build reverse lookup from name to SmartAttribute
    for (int id = 0x01; id <= 0xFE; ++id)
    {
        auto attr = static_cast<SmartData::SmartAttribute>(id);
        std::string name = SmartData::GetAttrTypeName(attr);
        if (name != "Unknown Attribute" && attrs.contains(name))
            attrs.at(name).get_to(s.smartData[attr]);
    }
}


// ProbeStatus
inline void to_json(nlohmann::json& j, const ProbeStatus& p)
{
    j = nlohmann::json{{"health", p.health}};

    if (p.rawData.index() == 0)
    {
        j["rawData"] = nlohmann::json{{"type", "text"}, {"value", std::get<0>(p.rawData)}};
    }
    else
    {
        nlohmann::json smartJson;
        to_json(smartJson, std::get<1>(p.rawData));
        smartJson["type"] = "smart";
        j["rawData"] = smartJson;
    }
}

inline void from_json(const nlohmann::json& j, ProbeStatus& p)
{
    j.at("health").get_to(p.health);

    const auto& rd = j.at("rawData");
    const auto type = rd.at("type").get<std::string>();

    if (type == "text")
    {
        p.rawData = rd.at("value").get<std::string>();
    }
    else if (type == "smart")
    {
        SmartData sd;
        from_json(rd, sd);
        p.rawData = sd;
    }
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
