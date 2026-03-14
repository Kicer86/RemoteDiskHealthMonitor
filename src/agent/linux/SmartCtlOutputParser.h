
#ifndef SMARTCTLOUTPUTPARSER_H
#define SMARTCTLOUTPUTPARSER_H

#include <string>
#include <map>

#include "common/SmartData.h"
#include "common/DiskSummary.h"

namespace SmartCtlOutputParser
{
    SmartData parse(const std::string &);
    SmartTestStatus parseTestStatus(const std::string &);
};

#endif
