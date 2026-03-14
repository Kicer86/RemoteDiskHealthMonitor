
#pragma once

#include <string>
#include <vector>
#include "GeneralHealth.h"
#include "ProbeStatus.h"

class DiskInfo
{
public:
	DiskInfo();

	DiskInfo(std::string _name, const GeneralHealth::Health& _health, const std::vector<ProbeStatus> &);

	void SetHealth(const GeneralHealth::Health& _health);
	void SetName(const std::string& _name);
    void SetProbesStatuses(const std::vector<ProbeStatus> &);

	const std::string& GetName() const;
	GeneralHealth::Health GetHealth() const;
    const std::vector<ProbeStatus>& GetProbesStatuses() const;

	bool operator==(const DiskInfo& _other) const = default;

private:
	std::string m_name;
	GeneralHealth::Health m_health;
    std::vector<ProbeStatus> m_statuses;
};


