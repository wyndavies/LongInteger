#pragma once

#ifndef _MYHARDWAREINFO_H_
#define _MYHARDWAREINFO_H_

#ifdef _WIN32
typedef BOOL(WINAPI *LPFN_GLPI)(
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION,
	PDWORD);
#elif MACOS
// This was taken from stackexchange. I don't have access to a MAC so no idea if this works
#include <sys/param.h>
#include <sys/sysctl.h>
#else
#include <unistd.h>
#include <thread>
#endif


class MyHardwareInfo
{
public:
	MyHardwareInfo();
	~MyHardwareInfo();

	int GetPhysicalCores();
	int GetLogicalCores();
	int GetCPUs();
private:

	int mPhysicalCores;
	int mLogicalCores;
	int mCPUs;

	int init();
#ifdef _WIN32
	DWORD CountSetBits(ULONG_PTR bitMask);
#endif
};

#endif
