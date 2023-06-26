
#ifndef AGENT_JSONUTILS_HPP_INCLUDED
#define AGENT_JSONUTILS_HPP_INCLUDED

#include <QString>

#include "SmartData.h"
#include "../common/DiskInfo.h"


namespace JSonUtils
{
    QString SmartDataToJSon(const SmartData &);
}


#endif
