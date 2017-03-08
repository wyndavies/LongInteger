#pragma once
/*
	This class is used to test the multi-threading routine. It will take in 2 parameters
	and use the parameters to construct a filename to write to the disk.
*/

#ifndef _WIN32
#include <thread>
#include <string>
using std::string;
#endif


class DummyCalledClass
{
public:
	DummyCalledClass();
	~DummyCalledClass();

#ifdef _WIN32
	CString createAFile(int param1, int param2);
	CString recursiveCall(int param1, int param2);
#else
	string createAFile(int param1, int param2);
	string recursiveCall(int param1, int param2);
#endif

};

