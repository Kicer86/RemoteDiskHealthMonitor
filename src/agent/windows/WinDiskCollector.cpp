#include "WinDiskCollector.h"
#include "WMICommunication.h"

std::vector<Disk> WinDiskCollector::GetDisksList()
{
    WMICommunication wmi;
    if (!wmi.WMIInit(WMICommunication::WmiNamespace::Discs))
        return {};

    if (!wmi.CollectInfoAboutDiscsViaWMI())
        return {};

    return wmi.GetDisksCollection();
}
