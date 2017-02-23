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
	LongInteger* param1;
	LongInteger* param2;
	LongInteger* pResult;
	ReceiveUpdateClass *callback;
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

	LongInteger* getResult();
	void setParams(LongInteger* pliParam1, LongInteger* pliParam2);

	// For testing
	void setDummyObject(DummyCalledClass& inpDCC);
	DummyCalledClass& getDummyObject();
	void setDummyParams(int, int);
	int getDummyParam1();
	int getDummyParam2();
	CString testValue;
};

