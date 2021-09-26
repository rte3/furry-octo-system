

#include "stdafx.h"
#include <Setupapi.h>
#include <Cfgmgr32.h>
#include "resource.h"
#include "FindUSBDeviceDlg.h"
#include "FindDevice.h"


BOOL FindDevice(int VID, int PID, CFindUSBDeviceDlg *pParent)
{
    DWORD dwSize, dwPropertyRegDataType;
    OSVERSIONINFO osvi;
    CONFIGRET cr;
    HDEVINFO hDevInfo;
    SP_DEVINFO_DATA DeviceInfoData;
    BOOL bShowAll = FALSE;
    char szDeviceInstanceID[MAX_PATH];
    char szSearchVID[16];
    char szSearchPID[16];
    char szTempVID[16];
    char szTempPID[16];
    char szTemp[16];
    char szPrevName[80] = "";
    int ret = FALSE;
    static const LPCTSTR szPrefix[3] = {"VID_", "PID_", "MI_"};

    sprintf(szSearchVID, "VID_%04X", VID);
    sprintf(szSearchPID, "PID_%04X", PID);

    hDevInfo = SetupDiGetClassDevs(NULL, TEXT("USB"), NULL, DIGCF_PRESENT|DIGCF_ALLCLASSES);
    if (hDevInfo == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    bShowAll = pParent->IsDlgButtonChecked(IDC_CHECK_SHOW_ALL);
    pParent->m_pList->ResetContent();

    for (int i = 0; ; i++)
    {
        BYTE temp[256];
        char *szToken;
        char *szCurrentToken;

        DeviceInfoData.cbSize = sizeof(DeviceInfoData);
        if (!SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData))
        {
            break;
        }

        // make sure device is on the local machine
        cr = CM_Get_Device_ID(DeviceInfoData.DevInst, szDeviceInstanceID , MAX_PATH, 0);
        if (cr != CR_SUCCESS)
        {
            continue;
        }

        SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_DEVICEDESC,
                                              &dwPropertyRegDataType, temp,
                                              sizeof(temp), 
                                              &dwSize);

        // Retreive the device description as reported by the device itself
        memset(&osvi, 0, sizeof(OSVERSIONINFO));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

        szToken = _tcstok_s(szDeviceInstanceID , TEXT("\\#&"), &szCurrentToken);
        *szTempVID = 0;
        *szTempPID = 0;
        *szTemp = 0;
        while (szToken != NULL)
        {
            for (int j = 0; j < 3; j++)
            {
                if (_tcsncmp(szToken, szPrefix[j], lstrlen(szPrefix[j])) == 0)
                {
                    switch (j)
                    {
                        case 0:
                            strcpy_s(szTempVID, sizeof(szTempVID), szToken);
                            break;
                        case 1:
                            strcpy_s(szTempPID, sizeof(szTempPID), szToken);
                            break;
                        case 2:
                            strcpy_s(szTemp, sizeof(szTemp), szToken);
                            break;
                        default:
                            break;
                    }
                }
            }
            szToken = _tcstok_s(NULL, TEXT("\\#&"), &szCurrentToken);

            if (bShowAll || stricmp(szSearchVID, szTempVID) == 0 && stricmp(szSearchPID, szTempPID) == 0)
            {
                SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_FRIENDLYNAME,
                                              &dwPropertyRegDataType, temp,
                                              sizeof(temp), 
                                              &dwSize);

                if ((bShowAll || strstr((char *) temp, "COM") != NULL) && strcmp(szPrevName, (char *) temp) != 0)
                {
                    pParent->m_pList->AddString((char *) temp);
                    strcpy(szPrevName, (char *) temp);
                }

                ret = TRUE;
            }
        }
    }

    return ret;
}
