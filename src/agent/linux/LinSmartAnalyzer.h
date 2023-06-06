
#pragma once

#include "../IProbe.h"

class LinSmartAnalyzer: public IProbe
{
public:
    GeneralHealth::Health GetStatus(const Disk& _disk) override;
    QString GetJSonData(const Disk& _disk) override;
};
