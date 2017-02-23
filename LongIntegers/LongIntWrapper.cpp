#include "stdafx.h"
#include "LongIntWrapper.h"
#include "DummyCalledClass.h"

QueueOfThreads LongIntWrapper::qot;

LongIntWrapper::LongIntWrapper()
{
	param1 = nullptr;
	param2 = nullptr;
	pResult = nullptr;
	bRunning = false;
	bFinished = false;
}


LongIntWrapper::~LongIntWrapper()
{
}

QueueOfThreads* LongIntWrapper::getQOT()
{
	return &qot;
}

int LongIntWrapper::startProcess()
{
	bRunning = true;

	pResult = LongInteger::karatsubaMain(param1, param2);


	// Test code
	//pDCC->recursiveCall(iParam1, iParam2);


//	pResult = pObject->karatsubaMain(param1, param2);
	// Dummy processing to add in a delay
/*	int j = 0;
	for (int i = 1; i < 1000000000; i++)
	{
		j = j + 1;
	}
*/


	bRunning = false;
	// Need a way this can inform QueueOfThreads that it has finished.
	callback->iHaveFinished(id);
	bFinished = true;
	return 0;
}

void LongIntWrapper::setCallback(ReceiveUpdateClass* newcallback)
{
	callback = newcallback;
}

void LongIntWrapper::setID(UINT newID)
{
	id = newID;
}

UINT LongIntWrapper::getID()
{
	return id;
}

void LongIntWrapper::setDummyParams(int inpar1, int inpar2)
{
	iParam1 = inpar1;
	iParam2 = inpar2;
}

int LongIntWrapper::getDummyParam1()
{
	return iParam1;
}

int LongIntWrapper::getDummyParam2()
{
	return iParam2;
}

void LongIntWrapper::setDummyObject(DummyCalledClass& inpDCC)
{
	pDCC = &inpDCC;
}

DummyCalledClass& LongIntWrapper::getDummyObject()
{
	return *pDCC;
}


LongInteger* LongIntWrapper::getResult()
{
	return pResult;
}

void LongIntWrapper::setParams(LongInteger* pliParam1, LongInteger* pliParam2)
{
	param1 = pliParam1;
	param2 = pliParam2;
}
