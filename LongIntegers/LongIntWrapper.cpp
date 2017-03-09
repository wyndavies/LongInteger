#ifdef _WIN32
#include "stdafx.h"
#endif
#include "LongIntWrapper.h"

QueueOfThreads<LongIntWrapper> LongIntWrapper::qot;

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
	delete param1;
	delete param2;
}

QueueOfThreads<LongIntWrapper>* LongIntWrapper::getQOT()
{
	return &qot;
}

int LongIntWrapper::startProcess()
{
	bRunning = true;

	pResult = LongInteger::karatsubaMain(*param1, *param2, true);


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
	bFinished = true;
	callback->iHaveFinished(id);
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

void LongIntWrapper::setParams(const LongInteger& liParam1, const LongInteger& liParam2)
{
	// Copy the parameters
	param1 = new LongInteger;
	param2 = new LongInteger;

	*param1 = liParam1;
	*param2 = liParam2;
}

