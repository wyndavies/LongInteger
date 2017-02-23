#pragma once
#include "LongInteger.h"
#include "ReceiveUpdateClass.h" // Interface
/*
 This code is specific to LongInteger. Get this working first before making a generic class
*/
#include "DummyCalledClass.h"


class LongInteger;
class QueueOfThreads;

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
	static QueueOfThreads qot; // Only 1 instance of QueueOfThreads

	// For testing
	DummyCalledClass* pDCC;
	int iParam1;
	int iParam2;

public:
	bool bRunning;
	bool bFinished;
	int startProcess();
	void setCallback(ReceiveUpdateClass*);
	void setID(UINT);
	UINT getID();
	static QueueOfThreads* getQOT();

	void setObject(DummyCalledClass& inpDCC);
	DummyCalledClass& getObject();
	void setParams(int, int);
	int getParam1();
	int getParam2();

	CString testValue;
};

