#include "stdafx.h"
#include "LongIntWrapper.h"
#include "DummyCalledClass.h"



LongIntWrapper::LongIntWrapper()
{
	pObject = nullptr;
	param1 = nullptr;
	param2 = nullptr;
	pResult = nullptr;
	bRunning = false;
	bFinished = false;
}


LongIntWrapper::~LongIntWrapper()
{
}

int LongIntWrapper::startProcess()
{
	bRunning = true;
//	pResult = pObject->karatsubaMain(param1, param2);
	// Dummy processing to add in a delay
	int j = 0;
	for (int i = 1; i < 1000000000; i++)
	{
		j = j + 1;
	}


	DummyCalledClass dcc;
	testValue = dcc.createAFile(id + 1, id + 1);


	bRunning = false;
	// Need a way this can inform QueueOfThreads that it has finished.
	callback->receiveUpdate(id);
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
