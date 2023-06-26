
#ifndef COMMON_JSONUTILS_HPP_INCLUDED
#define COMMON_JSONUTILS_HPP_INCLUDED

#include <QJsonArray>
#include <QJsonObject>

#include <QString>

#include "DiskInfo.h"


namespace JSonUtils
{
    QJsonArray DiskInfoToJSon(const std::vector<DiskInfo> &);
    std::vector<DiskInfo> JSonToDiskInfo(const QJsonArray &);
}


#endif
