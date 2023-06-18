
#include <magic_enum.hpp>

#include "SmartData.h"


void SmartData::add(SmartData::SmartAttribute attr, const SmartData::AttrData& value)
{
    m_smartData.emplace(attr, value);
}


const std::map<SmartData::SmartAttribute, SmartData::AttrData>& SmartData::data() const
{
    return m_smartData;
}


std::optional<QString> SmartData::GetAttrTypeName(const SmartAttribute& attr)
{
    if (magic_enum::enum_contains(attr))
    {
        const std::string name(magic_enum::enum_name(attr));
        return QString::fromStdString(name);
    }
    else
        return {};
}
