
#ifndef SMARTCTLOUTPUTPARSER_H
#define SMARTCTLOUTPUTPARSER_H

#include <QByteArray>
#include <map>

#include "../SmartData.h"


namespace SmartCtlOutputParser
{
    SmartData parse(const QByteArray &);
};

#endif
