
#ifndef SMARTCTLOUTPUTPARSER_H
#define SMARTCTLOUTPUTPARSER_H

#include <QByteArray>
#include <map>

#include "common/SmartData.h"

namespace SmartCtlOutputParser
{
    SmartData parse(const QByteArray &);
};

#endif
