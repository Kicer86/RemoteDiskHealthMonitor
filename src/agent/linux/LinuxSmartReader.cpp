
#include <QProcess>
#include <QDebug>

#include "../SmartReader.h"
#include "SmartCtlOutputParser.h"


SmartData SmartReader::ReadSMARTData(const Disk& disk)
{
    QProcess smartctl;

    const QString diskName = QString::fromStdString(disk.GetDeviceId());

    qDebug() << "Reading S.M.A.R.T. of"  << diskName;

    smartctl.start("smartctl", { "-a", "/dev/" + diskName }, QProcess::ReadOnly);
    const bool status = smartctl.waitForFinished(5000);

    qDebug() << "Finished with:" << status;

    const QByteArray output = smartctl.readAll();
    const auto entries = SmartCtlOutputParser::parse(output);

    return entries;
}


GeneralHealth SmartReader::ReadDisksStatus(const Disk &)
{
    return GeneralHealth::GOOD;
}
