#ifdef _WIN32
#include "stdafx.h"
#else
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
using std::string;
using std::ifstream;
#endif
#include "MyHardwareInfo.h"


MyHardwareInfo::MyHardwareInfo()
{
	if (init() != 0)
	{
		mPhysicalCores = 0;
		mLogicalCores = 0;
	}
}

MyHardwareInfo::~MyHardwareInfo()
{

}

int MyHardwareInfo::GetCPUCount()
{
	return mCPUs;
}

int MyHardwareInfo::GetPhysicalCores()
{
	return mPhysicalCores;
}


int MyHardwareInfo::GetLogicalCores()
{
	return mLogicalCores;
}


int MyHardwareInfo::init()
{
#ifdef _WIN32
	/* Not even going to pretend I understand this code.
	Got it from https://msdn.microsoft.com/en-us/library/ms683194
	*/

	LPFN_GLPI glpi;
	BOOL done = FALSE;
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
	DWORD returnLength = 0;
	DWORD logicalProcessorCount = 0;
	DWORD numaNodeCount = 0;
	DWORD processorCoreCount = 0;
	DWORD processorL1CacheCount = 0;
	DWORD processorL2CacheCount = 0;
	DWORD processorL3CacheCount = 0;
	DWORD processorPackageCount = 0;
	DWORD byteOffset = 0;
	PCACHE_DESCRIPTOR Cache;

	glpi = (LPFN_GLPI)GetProcAddress(
		GetModuleHandle(TEXT("kernel32")),
		"GetLogicalProcessorInformation");
	if (NULL == glpi)
	{
		CString error(TEXT("\nGetLogicalProcessorInformation is not supported.\n"));
		return (1);
	}

	while (!done)
	{
		DWORD rc = glpi(buffer, &returnLength);

		if (FALSE == rc)
		{
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				if (buffer)
					free(buffer);

				buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(
					returnLength);

				if (NULL == buffer)
				{
					CString error(TEXT("\nError: Allocation failure\n"));
					return (2);
				}
			}
			else
			{
				CString error(TEXT("\nError %d\n"), GetLastError());
				return (3);
			}
		}
		else
		{
			done = TRUE;
		}
	}

	ptr = buffer;

	while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength)
	{
		switch (ptr->Relationship)
		{
		case RelationNumaNode:
			// Non-NUMA systems report a single record of this type.
			numaNodeCount++;
			break;

		case RelationProcessorCore:
			processorCoreCount++;

			// A hyperthreaded core supplies more than one logical processor.
			logicalProcessorCount += CountSetBits(ptr->ProcessorMask);
			break;

		case RelationCache:
			// Cache data is in ptr->Cache, one CACHE_DESCRIPTOR structure for each cache. 
			Cache = &ptr->Cache;
			if (Cache->Level == 1)
			{
				processorL1CacheCount++;
			}
			else if (Cache->Level == 2)
			{
				processorL2CacheCount++;
			}
			else if (Cache->Level == 3)
			{
				processorL3CacheCount++;
			}
			break;

		case RelationProcessorPackage:
			// Logical processors share a physical package.
			processorPackageCount++;
			break;

		default:
			CString error(L"\nError: Unsupported LOGICAL_PROCESSOR_RELATIONSHIP value.\n");
			break;
		}
		byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
		ptr++;
	}

	free(buffer);

	mCPUs = processorPackageCount;
	mPhysicalCores = processorCoreCount;
	mLogicalCores = logicalProcessorCount;
#elif MACOS
	// This gets the number of logical cores on a MAC. Don't currently know
	// how to destinguish between logical and physical cores.
	// No clue if this works as I don't have a MAC to test it on
	int nm[2];
	size_t len = 4;
	uint32_t count;

	nm[0] = CTL_HW; nm[1] = HW_AVAILCPU;
	sysctl(nm, 2, &count, &len, NULL, 0);

	if (count < 1) {
		nm[1] = HW_NCPU;
		sysctl(nm, 2, &count, &len, NULL, 0);
		if (count < 1) { count = 1; }
		}
	mLogicalCores = count;
	mPhysicalCores = count;
#else
	mLogicalCores = std::thread::hardware_concurrency();

	// Struggling to find a way to get the number of physical cores
	// It is claimed this gives the info. Currently running in a VM
	// that has been created with 2 cores, no hyper-threading, so
	// no way to test this at the moment.
	// Will try on machine at home tomorrow.
	mPhysicalCores = sysconf(_SC_NPROCESSORS_ONLN);

	int numCPUs = 0;

	string line;
	string tmp1, tmp2, tmp3, strCPUID;
	string findText = "physical id";
	std::size_t found;
	std::string::size_type sz;
	std::vector<int> listOfIDs;
	ifstream myFile("/proc/cpuinfo");
	if (myFile.is_open())
	{
		while (std::getline(myFile, line))
		{
			found = line.find(findText);
			if (found != string::npos)
			{
				std::stringstream strStream(line);
				strStream >> tmp1 >> tmp2 >> tmp3 >> strCPUID;

				int iCPUID = std::stoi(strCPUID, &sz);

				// Find if this ID already exists in the vector
				bool bCPUFound = false;
				for (int ID : listOfIDs)
				{
					if (ID == iCPUID) bCPUFound = true;
				}
				if (!bCPUFound)
				{
					listOfIDs.push_back(iCPUID);
					numCPUs++;
				}
			}
		}
	}
	myFile.close();

	if (numCPUs == 0) numCPUs = 1; // Default to 1 if we can't find out how many

#endif
	return 0;
}

#ifdef _WIN32
// Helper function to count set bits in the processor mask.
DWORD MyHardwareInfo::CountSetBits(ULONG_PTR bitMask)
{
	DWORD LSHIFT = sizeof(ULONG_PTR) * 8 - 1;
	DWORD bitSetCount = 0;
	ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;
	DWORD i;

	for (i = 0; i <= LSHIFT; ++i)
	{
		bitSetCount += ((bitMask & bitTest) ? 1 : 0);
		bitTest /= 2;
	}

	return bitSetCount;
}
#endif
