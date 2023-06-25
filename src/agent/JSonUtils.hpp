
#ifndef JSONUTILS_HPP_INCLUDED
#define JSONUTILS_HPP_INCLUDED

#include <QJsonArray>
#include <QJsonObject>

#include <QString>

#include "SmartData.h"
#include "../common/DiskInfo.h"


namespace JSonUtils
{
    QString SmartDataToJSon(const SmartData &);
    QJsonArray DiskInfoToJSon(const std::vector<DiskInfo> &);
}


#endif // JSONUTILS_HPP_INCLUDED
