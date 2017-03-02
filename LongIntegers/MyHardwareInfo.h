#pragma once

#ifdef _WIN32
#include <windows.h>
#include <malloc.h>    
#include <stdio.h>
#include <tchar.h>
#elif MACOS
#include <sys/param.h>
#include <sys/sysctl.h>
#else
#include <unistd.h>
#endif


typedef BOOL(WINAPI *LPFN_GLPI)(
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION,
	PDWORD);

class MyHardwareInfo
{
public:
	MyHardwareInfo();
	~MyHardwareInfo();

	int GetPhysicalCores();
	int GetLogicalCores();
private:

	int mPhysicalCores;
	int mLogicalCores;

	int init();
	DWORD CountSetBits(ULONG_PTR bitMask);
	


};

