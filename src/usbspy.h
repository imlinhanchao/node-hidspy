#ifndef USBSPY_H
#define USBSPY_H

#include <nan.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <dbt.h>
#include <tchar.h>
#include <algorithm>

#define MAX_THREAD_WINDOW_NAME 64

#define _TEST_NODE_

typedef enum _DEVICE_STATUS{
	PULLOUT, PLUGIN
}DEVICE_STATUS;

typedef struct Device_t
{
	DEVICE_STATUS status;
	unsigned long dwPID;
	unsigned long dwVID;
	unsigned long dwInterface;
	char szGuid[37];
	char szDeviceName[260];
} Deivce, *PDeivce;

DWORD WINAPI SpyingThread();
LRESULT CALLBACK SpyCallback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void StartSpying();

#endif
