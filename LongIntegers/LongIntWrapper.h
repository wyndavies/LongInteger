#pragma once
#include "LongInteger.h"
#include "ReceiveUpdateClass.h" // Interface
/*
 This code is specific to LongInteger. Get this working first before making a generic class
*/


class LongInteger;

class LongIntWrapper
{
public:
	LongIntWrapper();
	~LongIntWrapper();

private:
	UINT id;
	LongInteger* pObject;
	LongInteger* param1;
	LongInteger* param2;
	LongInteger* pResult;
	ReceiveUpdateClass *callback; // Do this with generics once I actually get it working (strange compiler errors when I used <function>

public:
	bool bRunning;
	bool bFinished;
	int startProcess();
	void setCallback(ReceiveUpdateClass*);
	void setID(UINT);
	UINT getID();

	CString testValue;
};

