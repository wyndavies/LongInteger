#pragma once

#ifdef _WIN32
typedef BOOL(WINAPI *LPFN_GLPI)(
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION,
	PDWORD);
#elif MACOS
#include <sys/param.h>
#include <sys/sysctl.h>
#else
#include <unistd.h>
#endif


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
#ifdef _WIN32
	DWORD CountSetBits(ULONG_PTR bitMask);
#endif


};

