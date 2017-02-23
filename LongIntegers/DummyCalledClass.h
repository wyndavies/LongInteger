#pragma once
/*
	This class is used to test the multi-threading routine. It will take in 2 parameters
	and use the parameters to construct a filename to write to the disk.
*/

class DummyCalledClass
{
public:
	DummyCalledClass();
	~DummyCalledClass();

	CString createAFile(int param1, int param2);
	CString recursiveCall(int param1, int param2);

};

