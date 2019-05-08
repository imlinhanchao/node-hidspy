#include "usbspy.h"
#include <string>
extern "C" {  
#include "setupapi.h" 
#include "hidsdi.h" 
}
#pragma comment(lib, "hid.lib")

using namespace Nan;
using namespace std;

const typename AsyncProgressQueueWorker<Deivce>::ExecutionProgress *globalProgress;

char * _strcpy(const char* szSrc, char* szDst) 
{
	while(*szSrc){ 
		*szDst++ = *szSrc++;
	}
	*szDst = 0;
	return szDst;
}

void processData(const typename AsyncProgressQueueWorker<Deivce>::ExecutionProgress &progress)
{
	globalProgress = &progress;

	std::thread worker(SpyingThread);

#ifdef _DEBUG
	worker.join();
#else
	worker.detach();
#endif
}

DWORD WINAPI SpyingThread()
{
	char className[MAX_THREAD_WINDOW_NAME];
	_snprintf_s(className, MAX_THREAD_WINDOW_NAME, "ListnerThreadHidDetection_%d", GetCurrentThreadId());

	WNDCLASSA wincl = {0};
	wincl.hInstance = GetModuleHandle(0);
	wincl.lpszClassName = className;
	wincl.lpfnWndProc = SpyCallback;

	if (!RegisterClassA(&wincl))
	{
		DWORD le = GetLastError();
		printf("RegisterClassA() failed [Error: %x]\r\n", le);
		return 1;
	}

	HWND hwnd = CreateWindowExA(WS_EX_TOPMOST, className, className, 0, 0, 0, 0, 0, NULL, 0, 0, 0);
	if (!hwnd)
	{
		DWORD le = GetLastError();
		printf("CreateWindowExA() failed [Error: %x]\r\n", le);
		return 1;
	}

    DEV_BROADCAST_DEVICEINTERFACE broadcastInterface;
	broadcastInterface.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	broadcastInterface.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	HidD_GetHidGuid(&broadcastInterface.dbcc_classguid);
	HDEVNOTIFY hDevNotify = RegisterDeviceNotification(hwnd, &broadcastInterface, DEVICE_NOTIFY_WINDOW_HANDLE);

	if (!hDevNotify)
	{
		DWORD le = GetLastError();
		printf("RegisterDeviceNotificationA() failed [Error: %x]\r\n", le);
		return 1;
	}

	MSG msg;
	while (TRUE)
	{
		BOOL bRet = GetMessage(&msg, hwnd, 0, 0);
		if ((bRet == 0) || (bRet == -1))
		{
			break;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT CALLBACK SpyCallback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_DEVICECHANGE)
	{
        DEV_BROADCAST_DEVICEINTERFACE* dev = (DEV_BROADCAST_DEVICEINTERFACE*)lParam;

        if (NULL == dev)
            return FALSE;

        if (dev->dbcc_devicetype != DBT_DEVTYP_DEVICEINTERFACE)
            return FALSE;

        string sDevice = dev->dbcc_name;
        string sVID = sDevice.substr(sDevice.find("VID_") + 4, 4);
        string sPID = sDevice.substr(sDevice.find("PID_") + 4, 4);
        string sInterface = sDevice.substr(sDevice.find("{") - 5, 4);

        Deivce info;
        info.dwVID = strtol(sVID.c_str(), NULL, 16);
        info.dwPID = strtol(sPID.c_str(), NULL, 16);
        info.dwInterface = strtol(sInterface.c_str(), NULL, 16);
        GUID Guid = dev->dbcc_classguid;
        _strcpy(sDevice.c_str(), info.szDeviceName);

        char buff[100];
        snprintf(buff, sizeof(buff), "%08x-%04x-%04x-%02x%02x%02x%02x%02x%02x", 
            Guid.Data1, Guid.Data2, Guid.Data3, 
            Guid.Data4[2], Guid.Data4[3], Guid.Data4[4], Guid.Data4[5], 
            Guid.Data4[6], Guid.Data4[7]);

        _strcpy(buff, info.szGuid);

        //if(g_VID != 0 && info.dwVID != g_VID) return TRUE;

        switch(wParam)
        {
            // Device been plugged in and useful
            case DBT_DEVICEARRIVAL: 
                info.status = PLUGIN;
                globalProgress->Send(&info, 1);
                break; 
                // Device been removed
            case DBT_DEVICEREMOVECOMPLETE: 
                info.status = PULLOUT;
                globalProgress->Send(&info, 1);
                break;
            case DBT_DEVICEQUERYREMOVE: break;
            case DBT_DEVICEQUERYREMOVEFAILED: break;
            case DBT_DEVICEREMOVEPENDING: break;
            case DBT_DEVICETYPESPECIFIC: break;
            case DBT_CONFIGCHANGED: break;
            case DBT_DEVNODES_CHANGED: break;
            default: break;
        }
	}

	return 1;
}