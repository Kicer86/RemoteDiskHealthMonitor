#pragma once

#include <cstdint>
#include <string>


class GeneralHealth
{
public:
    enum Health : uint8_t
    {
        UNKNOWN = 0,
        GOOD,
        CHECK_STATUS,
        BAD
    };

    GeneralHealth();
    GeneralHealth(const Health& _health);
    GeneralHealth(const GeneralHealth& _health) = default;
    GeneralHealth& operator=(const GeneralHealth& _health) = default;

    Health GetStatus() const;

    void SetStatus(Health _health);

private:
    Health m_health;
};

std::string healthToString(GeneralHealth::Health health);
GeneralHealth::Health healthFromString(const std::string& str);
