#include "WMICommunication.h"
#include "CMDCommunication.h"

#define _WIN32_DCOM
#include <iostream>
#include <map>
#include <comdef.h>

WMICommunication::~WMICommunication()
{
    ReleaseComObjects();
    if (m_comInitialized)
        CoUninitialize();
}

bool WMICommunication::WMIInit(const WmiNamespace _namespace)
{
    ReleaseComObjects();

    HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
        return false;

    m_comInitialized = true;

    hres = CoInitializeSecurity(
        NULL,
        -1,                          // COM authentication
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_PKT,       // Default authentication
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities
        NULL                         // Reserved
    );

    if (FAILED(hres) && hres != RPC_E_TOO_LATE)
        return false;

    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&m_initialLocatorToWMI);

    if (FAILED(hres))
        return false;

    _bstr_t wmiNamespace;
    if (_namespace == Smart)
        wmiNamespace = L"ROOT\\WMI";
    else if (_namespace == Discs)
        wmiNamespace = L"ROOT\\cimv2";

    hres = m_initialLocatorToWMI->ConnectServer(
        wmiNamespace,              // Object path of WMI namespace
        NULL,                    // User name. NULL = current user
        NULL,                    // User password. NULL = current
        0,                       // Locale. NULL indicates current
        NULL,                    // Security flags.
        0,                       // Authority (for example, Kerberos)
        0,                       // Context object
        &m_services                // pointer to IWbemServices proxy
    );

    if (FAILED(hres))
        return false;

    hres = CoSetProxyBlanket(
        m_services,                    // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
        NULL,                        // Server principal name
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
        NULL,                        // client identity
        EOAC_NONE                    // proxy capabilities
    );

    if (FAILED(hres))
        return false;

    return true;
}

bool WMICommunication::CollectSMARTDataViaWMI(const Disk& _disk)
{
    if (m_services == nullptr)
        return false;

    try {
        ReleaseEnumerator();
        HRESULT hres = m_services->ExecQuery(
            bstr_t("WQL"),
            bstr_t("SELECT * FROM MSStorageDriver_FailurePredictData"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            NULL,
            &m_pEnumerator);

        if (FAILED(hres))
        {
            throw std::exception("Query for operating system name failed. Error code = 0x");
        }

        IWbemClassObject* pclsObj = NULL;
        ULONG uReturn = 0;

        while (m_pEnumerator)
        {
            HRESULT hr = m_pEnumerator->Next(WBEM_INFINITE, 1,
                &pclsObj, &uReturn);

            if (0 == uReturn)
            {
                break;
            }

            VARIANT vtInstanceName;
            VariantInit(&vtInstanceName);
            hr = pclsObj->Get(L"InstanceName", 0, &vtInstanceName, 0, 0);
            if (FAILED(hr) || vtInstanceName.vt != VT_BSTR)
            {
                VariantClear(&vtInstanceName);
                pclsObj->Release();
                continue;
            }

            CMDCommunication communicator;
            std::wstring instanceName = (vtInstanceName.bstrVal);
            if ( communicator.CompareDeviceIdWithInstanceName( _disk, std::string( instanceName.begin(), instanceName.end() ) ) )
            {
                VARIANT vtProp;
                VariantInit(&vtProp);
                hr = pclsObj->Get(L"VendorSpecific", 0, &vtProp, 0, 0);

                if (V_ISARRAY(&vtProp))
                {
                    LPSAFEARRAY pSafeArray = V_ARRAY(&vtProp);

                    VARTYPE itemType;
                    if (SUCCEEDED(SafeArrayGetVartype(pSafeArray, &itemType)))
                    {
                        if (itemType == VT_UI1)
                        {
                            if (SafeArrayGetDim(pSafeArray) == 1)
                            {
                                LONG lBound;
                                LONG uBound;

                                if (SUCCEEDED(SafeArrayGetLBound(pSafeArray, 1, &lBound)) && SUCCEEDED(SafeArrayGetUBound(pSafeArray, 1, &uBound)))
                                {
                                    const LONG itemCount = uBound - lBound + 1;

                                    std::vector<BYTE> dataFromArray(itemCount);

                                    BYTE* safearrayData;
                                    hr = SafeArrayAccessData(pSafeArray, reinterpret_cast<LPVOID*>(&safearrayData));
                                    if (FAILED(hr))
                                    {
                                        dataFromArray.clear();
                                    }

                                    memcpy(dataFromArray.data(), safearrayData, itemCount);

                                    hr = SafeArrayUnaccessData(pSafeArray);
                                    if (FAILED(hr))
                                    {
                                        dataFromArray.clear();
                                    }

                                    if (dataFromArray.empty() != true)
                                    {
                                        FeedSmartDataStructure(dataFromArray, itemCount);
                                    }
                                }
                            }
                        }
                    }
                }


                VariantClear(&vtProp);
            }
            VariantClear(&vtInstanceName);
            pclsObj->Release();
        }

        return true;
    }
    catch (const std::exception& e)
    {
        return false;
    }
}

bool WMICommunication::CollectInfoAboutDiscsViaWMI()
{
    if (m_services == nullptr)
        return false;

    try {
        ReleaseEnumerator();
        HRESULT hres = m_services->ExecQuery(
            bstr_t("WQL"),
            bstr_t("SELECT * FROM Win32_DiskDrive"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            NULL,
            &m_pEnumerator);

        if (FAILED(hres))
        {
            throw std::exception("Query for operating system name failed. Error code = 0x");
        }

        IWbemClassObject* pclsObj = NULL;
        ULONG uReturn = 0;

        while (m_pEnumerator)
        {
            HRESULT hr = m_pEnumerator->Next(WBEM_INFINITE, 1,
                &pclsObj, &uReturn);

            if (0 == uReturn)
            {
                break;
            }

            VARIANT vtPropDeviceId;
            VariantInit(&vtPropDeviceId);
            hr = pclsObj->Get(L"DeviceID", 0, &vtPropDeviceId, 0, 0);

            VARIANT vtPropModel;
            VariantInit(&vtPropModel);
            hr = pclsObj->Get(L"Model", 0, &vtPropModel, 0, 0);

            VARIANT vtPropSize;
            VariantInit(&vtPropSize);
            hr = pclsObj->Get(L"Size", 0, &vtPropSize, 0, 0);

            VARIANT vtPropInterfaceType;
            VariantInit(&vtPropInterfaceType);
            hr = pclsObj->Get(L"InterfaceType", 0, &vtPropInterfaceType, 0, 0);

            if (vtPropDeviceId.vt != VT_BSTR)
            {
                VariantClear(&vtPropInterfaceType);
                VariantClear(&vtPropSize);
                VariantClear(&vtPropModel);
                VariantClear(&vtPropDeviceId);
                pclsObj->Release();
                continue;
            }

            std::string model;
            if (vtPropModel.vt == VT_BSTR)
                model = StringFromVariant(vtPropModel);

            uint64_t capacity = 0;
            if (vtPropSize.vt != VT_NULL)
                capacity = Uint64FromVariant(vtPropSize);

            std::string driveType;
            if (model.find("NVMe") != std::string::npos || model.find("nvme") != std::string::npos)
                driveType = "NVMe";
            else if (vtPropInterfaceType.vt == VT_BSTR && StringFromVariant(vtPropInterfaceType) == "USB")
                driveType = "USB";

            Disk disc( StringFromVariant(vtPropDeviceId), model, capacity, driveType );

            m_discsCollection.push_back(disc);

            VariantClear(&vtPropInterfaceType);
            VariantClear(&vtPropSize);
            VariantClear(&vtPropModel);
            VariantClear(&vtPropDeviceId);

            pclsObj->Release();
        }

        return true;
    }
    catch (const std::exception& e)
    {
        return false;
    }
}

const SmartData& WMICommunication::GetSMARTData() const
{
    return m_smartData;
}

const std::vector<Disk> WMICommunication::GetDisksCollection() const
{
    return m_discsCollection;
}

void WMICommunication::FeedSmartDataStructure(const std::vector<BYTE>& _data, const LONG& _dataSize)
{

    for (int i = 0; _data.size() > i; i += 12)
    {
        if (_data.size() >= (i + 12))
        {
            uint8_t id = _data.at(i + 2);
            int64_t rawVal = 0;
            for (int b = 5; b >= 0; --b)
                rawVal = (rawVal << 8) | _data.at(i + 5 + b);

            m_smartData.attributes.push_back(SmartData::Attribute{
                id,
                SmartData::GetCanonicalName(id),
                static_cast<int>(_data.at(i + 3)),
                static_cast<int>(_data.at(i + 4)),
                0,
                rawVal
            });
        }

    }

}

bool WMICommunication::CollectThresholdsViaWMI(const Disk& _disk)
{
    if (m_services == nullptr)
        return false;

    try {
        ReleaseEnumerator();
        HRESULT hres = m_services->ExecQuery(
            bstr_t("WQL"),
            bstr_t("SELECT * FROM MSStorageDriver_FailurePredictThresholds"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            NULL,
            &m_pEnumerator);

        if (FAILED(hres))
            return false;

        IWbemClassObject* pclsObj = NULL;
        ULONG uReturn = 0;

        while (m_pEnumerator)
        {
            HRESULT hr = m_pEnumerator->Next(WBEM_INFINITE, 1,
                &pclsObj, &uReturn);

            if (0 == uReturn)
                break;

            VARIANT vtInstanceName;
            VariantInit(&vtInstanceName);
            hr = pclsObj->Get(L"InstanceName", 0, &vtInstanceName, 0, 0);
            if (FAILED(hr) || vtInstanceName.vt != VT_BSTR)
            {
                VariantClear(&vtInstanceName);
                pclsObj->Release();
                continue;
            }

            CMDCommunication communicator;
            std::wstring instanceName = (vtInstanceName.bstrVal);
            if (communicator.CompareDeviceIdWithInstanceName(_disk, std::string(instanceName.begin(), instanceName.end())))
            {
                VARIANT vtProp;
                VariantInit(&vtProp);
                hr = pclsObj->Get(L"VendorSpecific", 0, &vtProp, 0, 0);

                if (V_ISARRAY(&vtProp))
                {
                    LPSAFEARRAY pSafeArray = V_ARRAY(&vtProp);

                    VARTYPE itemType;
                    if (SUCCEEDED(SafeArrayGetVartype(pSafeArray, &itemType)) && itemType == VT_UI1
                        && SafeArrayGetDim(pSafeArray) == 1)
                    {
                        LONG lBound, uBound;
                        if (SUCCEEDED(SafeArrayGetLBound(pSafeArray, 1, &lBound))
                            && SUCCEEDED(SafeArrayGetUBound(pSafeArray, 1, &uBound)))
                        {
                            const LONG itemCount = uBound - lBound + 1;
                            std::vector<BYTE> data(itemCount);

                            BYTE* safearrayData;
                            hr = SafeArrayAccessData(pSafeArray, reinterpret_cast<LPVOID*>(&safearrayData));
                            if (SUCCEEDED(hr))
                            {
                                memcpy(data.data(), safearrayData, itemCount);
                                SafeArrayUnaccessData(pSafeArray);
                                FeedThresholds(data, itemCount);
                            }
                        }
                    }
                }

                VariantClear(&vtProp);
            }

            VariantClear(&vtInstanceName);
            pclsObj->Release();
        }

        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

void WMICommunication::FeedThresholds(const std::vector<BYTE>& _data, const LONG& _dataSize)
{
    // Build a map of attribute ID → threshold value from the blob.
    // Layout follows the same 12-byte chunking as FeedSmartDataStructure:
    // byte[i+2] = attribute ID, byte[i+3] = threshold value.
    std::map<uint8_t, int> thresholds;
    for (size_t i = 0; i + 12 <= static_cast<size_t>(_dataSize); i += 12)
    {
        uint8_t id = _data[i + 2];
        if (id != 0)
            thresholds[id] = static_cast<int>(_data[i + 3]);
    }

    for (auto& attr : m_smartData.attributes)
    {
        if (auto it = thresholds.find(attr.id); it != thresholds.end())
            attr.threshold = it->second;
    }
}

void WMICommunication::ReleaseEnumerator()
{
    if (m_pEnumerator != nullptr)
    {
        m_pEnumerator->Release();
        m_pEnumerator = nullptr;
    }
}

void WMICommunication::ReleaseComObjects()
{
    ReleaseEnumerator();

    if (m_services != nullptr)
    {
        m_services->Release();
        m_services = nullptr;
    }

    if (m_initialLocatorToWMI != nullptr)
    {
        m_initialLocatorToWMI->Release();
        m_initialLocatorToWMI = nullptr;
    }
}

std::string WMICommunication::StringFromVariant(VARIANT& vt)
{
        _bstr_t bs(vt);
        return std::string(static_cast<const char*>(bs));
}

uint64_t WMICommunication::Uint64FromVariant(VARIANT& vt)
{
    if (vt.vt == VT_BSTR)
    {
        try { return std::stoull(StringFromVariant(vt)); }
        catch (...) { return 0; }
    }
    if (vt.vt == VT_I4)  return static_cast<uint64_t>(vt.lVal);
    if (vt.vt == VT_UI4) return static_cast<uint64_t>(vt.ulVal);
    if (vt.vt == VT_I8)  return static_cast<uint64_t>(vt.llVal);
    if (vt.vt == VT_UI8) return vt.ullVal;
    return 0;
}

