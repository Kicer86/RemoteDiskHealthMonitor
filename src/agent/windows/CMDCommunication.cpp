#include "CMDCommunication.h"
#include <fstream>
#include <iostream>
#include <QProcess>
#include <algorithm>

GeneralHealth::Health CMDCommunication::CollectDiskStatus(const Disk& _disk)
{
    const std::string commandResult = ExecuteDiscStatusCommand(_disk);

    if (commandResult == "OK")
        return GeneralHealth::GOOD;
    else if (commandResult == "Degraded")
        return GeneralHealth::CHECK_STATUS;
    else if (commandResult == "PredFail")
        return GeneralHealth::BAD;

    return GeneralHealth::UNKNOWN;
}

bool CMDCommunication::CompareDeviceIdWithInstanceName(const Disk& _disk, std::string _instanceName)
{
    std::string diskInstanceName = GetInstanceName(_disk);

    auto diskPos = _instanceName.find("_0");
    _instanceName = _instanceName.substr(0, diskPos);

    ChangeStringToLowerCase(_instanceName);
    ChangeStringToLowerCase(diskInstanceName);

    return ( diskInstanceName.compare(_instanceName) == 0 );
}

std::string CMDCommunication::ExecuteDiscStatusCommand(const Disk& _disk) const
{
    QProcess proc;
    QString program = "wmic";
    QStringList arguments;
    arguments << "diskdrive" << "get" << "deviceid,status";
    proc.start(program, arguments);
    proc.waitForFinished();
    auto output = proc.readAllStandardOutput();

    std::string ret = output.toStdString();

    auto diskPos = ret.find(_disk.GetDeviceId());
    auto statusPosStart = ret.find_first_not_of(' ', diskPos + (_disk.GetDeviceId()).size());
    auto statusPosStop = ret.find_first_of(' ', statusPosStart);
    ret = ret.substr(statusPosStart, statusPosStop - statusPosStart);
    return ret;
}

std::string CMDCommunication::GetInstanceName(const Disk& _disk) const
{
    QProcess proc;
    QString program = "wmic";
    QStringList arguments;
    arguments << "diskdrive" << "get" << "DeviceID," << "PNPDeviceID";
    proc.start(program, arguments);
    proc.waitForFinished();
    QString output = proc.readAllStandardOutput();

    std::string ret = output.toStdString();

    auto diskPos = ret.find(_disk.GetDeviceId());
    auto statusPosStart = ret.find_first_not_of(' ', diskPos + (_disk.GetDeviceId()).size());
    auto statusPosStop = ret.find_first_of(' ', statusPosStart);
    ret = ret.substr(statusPosStart, statusPosStop - statusPosStart);
    return ret;
}

void CMDCommunication::ChangeStringToLowerCase(std::string& _string) const
{
    std::transform(_string.begin(), _string.end(), _string.begin(), ::tolower);
}
