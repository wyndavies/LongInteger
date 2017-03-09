/*
LongInteger.h

This class is a wrapper to LongInteger. It allows LongIntegers to be sent to QueueOfThreads and then collects
the results of processing to be picked up by the caller.
It has been created both to separate LongInteger and QueueOfThreads and also with a mind to creating a generic
wrapper to allow for different items to be sent for multi-threading.

At the moment it just calls the karatsuba algorithm. Will need to be amended to allow for calling ToomCook3 and
BurnikelZeigler.

*/

#pragma once
#include "LongInteger.h"
#include "ReceiveUpdateClass.h"
#include "QueueOfThreads.h"
/*
This code is specific to LongInteger. Get this working first before making a generic class
*/
#include "DummyCalledClass.h"

#ifndef _WIN32
// I'm not sure why I have to include this typedef again as it is in LongInteger.h
typedef unsigned int UINT;
#endif

class LongInteger;
//class QueueOfThreads;

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
	static QueueOfThreads<LongIntWrapper> qot; // Only 1 instance of QueueOfThreads

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
	static QueueOfThreads<LongIntWrapper>* getQOT();

	LongInteger* getResult();
	//	void setParams(LongInteger* pliParam1, LongInteger* pliParam2);
	void setParams(const LongInteger& liParam1, const LongInteger& liParam2);

	// For testing
	void setDummyObject(DummyCalledClass& inpDCC);
	DummyCalledClass& getDummyObject();
	void setDummyParams(int, int);
	int getDummyParam1();
	int getDummyParam2();
#ifdef _WIN32
	CString testValue;
#else
	string testValue;
#endif
};

