#include "WMICommunication.h"
#include "WMICommunication.h"
#include "WMICommunication.h"

#define _WIN32_DCOM
#include <iostream>
#include <comdef.h>
#pragma comment(lib, "wbemuuid.lib")

#include <vector>

WMICommunication::WMICommunication()
{
}

WMICommunication::~WMICommunication()
{
    initialLocatorToWMI->Release();
    services->Release();
    pEnumerator->Release();
    CoUninitialize();
}

bool WMICommunication::WMIInit()
{
    HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);

    if (FAILED(hres))
    {
        throw std::exception("Failed to initialize COM library. Error code = 0x");
        return false;
    }

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


    if (FAILED(hres))
    {
        CoUninitialize();
        throw std::exception("Failed to initialize security. Error code = 0x");
        return false;
    }



    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&initialLocatorToWMI);

    if (FAILED(hres))
    {
        CoUninitialize();
        throw std::exception("Failed to create IWbemLocator object. Err code = 0x");

        return false;
    }

    hres = initialLocatorToWMI->ConnectServer(
        _bstr_t(L"ROOT\\WMI"),   // Object path of WMI namespace
        NULL,                    // User name. NULL = current user
        NULL,                    // User password. NULL = current
        0,                       // Locale. NULL indicates current
        NULL,                    // Security flags.
        0,                       // Authority (for example, Kerberos)
        0,                       // Context object 
        &services                // pointer to IWbemServices proxy
    );

    if (FAILED(hres))
    {
        initialLocatorToWMI->Release();
        CoUninitialize();
        throw std::exception("Could not connect. Error code = 0x");
        return false;
    }


    hres = CoSetProxyBlanket(
        services,                    // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
        NULL,                        // Server principal name 
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
        NULL,                        // client identity
        EOAC_NONE                    // proxy capabilities 
    );

    if (FAILED(hres))
    {

        services->Release();
        initialLocatorToWMI->Release();
        CoUninitialize();
        throw std::exception("Could not set proxy blanket. Error code = 0x");
        return false;
    }

	return true;
}

bool WMICommunication::GetSMARTDataViaWMI()
{

    HRESULT hres = services->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM MSStorageDriver_FailurePredictData"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hres))
    {
        services->Release();
        services->Release();
        CoUninitialize();
        throw std::exception("Query for operating system name failed. Error code = 0x");
        return false;
    }

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
            &pclsObj, &uReturn);

        if (0 == uReturn)
        {
            break;
        }

        VARIANT vtProp;
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
                            LONG itemCount = uBound - lBound + 1;

                            BYTE* pData = new BYTE[itemCount];

                            BYTE* safearrayData;
                            hr = SafeArrayAccessData(pSafeArray, reinterpret_cast<LPVOID*>(&safearrayData));
                            if (FAILED(hr))
                            {
                                delete[] pData;
                            }

                            memcpy(pData, safearrayData, itemCount);

                            hr = SafeArrayUnaccessData(pSafeArray);
                            if (FAILED(hr))
                            {
                                delete[] pData;
                            }

                            if (pData != NULL)
                            {
                                FeedSmartDataStructure(pData, itemCount);
                                std::cout << "\nA TO WYNIK: " << dataVector.size() << std::endl;
                            }
                        }
                    }
                }
            }
        }
       
        VariantClear(&vtProp);

        pclsObj->Release();
    }
}

void WMICommunication::FeedSmartDataStructure(BYTE* data, int dataSize)
{
    for (int i = 0; i < dataSize; ++i)
    {
        dataVector.push_back(data[i]);
    }
}
