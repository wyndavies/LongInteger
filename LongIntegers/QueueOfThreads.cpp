#include "stdafx.h"
#include "QueueOfThreads.h"


QueueOfThreads::QueueOfThreads()
{
	threadsRunning = 0;
	threadsWaiting = 0;
	threadID = 0;
	maxThreads = std::thread::hardware_concurrency();
	if (maxThreads < minThreads) {
		maxThreads = minThreads; // Default value
	}
}


QueueOfThreads::~QueueOfThreads()
{
	// Can't exit whilst threads are running.
	// Set the shutting down flag (all should have a reference to that - use an interface)
	// and wait for them to finish
	
}

void QueueOfThreads::decreaseCount(UINT id)
{
	try {
	std::unique_lock<std::mutex> lk(lock);
	// Find out which thread has finished and remove it from the list
	UINT index = 0;
	bool bFound = false;
	while (!bFound && index < queueOfRunningThreads.size())
	{
		if (queueOfRunningThreads[index]->getID() == id)
		{
			bFound = true;
		}
		else
		{
			index++;
		}
	}

	if (!bFound) abort();

	LongIntWrapper* finishedLIW = queueOfRunningThreads[index];
	if (finishedLIW == nullptr)
	{
		abort();
	}

	queueOfRunningThreads.erase(queueOfRunningThreads.begin() + index);
	threadsRunning--;
	lk.unlock();
	cv.notify_one();

	}
	catch (std::exception e)
	{
		CString fred(e.what());
	}
	catch (...)
	{
		CString fred(L"Dunno what happened");
	}

}

bool QueueOfThreads::addToQueue(LongIntWrapper* newLongInt)
{
	try {
	
	std::unique_lock<std::mutex> lk(lock);

	newLongInt->setID(threadID);
	queueOfWaitingThreads.push_back(newLongInt);
	threadID++;
	threadsWaiting++;
	newLongInt->setCallback(this);
	// I was using a lambda and it was working fine, but this format of thread starting
	// matches up to the other one.
	// Starting a thread to start a thread may seem weird, but this functionality is just a placeholder
	std::thread t1(&QueueOfThreads::startAThread, this);
	t1.detach();
	lk.unlock();
	}
	catch (std::exception e)
	{
		CString fred(e.what());
	}
	catch (...)
	{
		CString fred(L"Dunno what happened");
	}
	return true;
}

void QueueOfThreads::startAThread()
{
	if (threadsWaiting == 0)
	{
		return;
	}
	try {
	std::unique_lock<std::mutex> lk(lock);
	// The wait will unlock and then try to reacquire the lock when it wakes up
	cv.wait(lk, [this]() { return threadsRunning < maxThreads; });


	// Insert code to start a thread
	if (queueOfWaitingThreads.size() == 0)
	{
		abort();
	}

	
	LongIntWrapper* tempLIW = queueOfWaitingThreads[0];
	queueOfWaitingThreads.erase(queueOfWaitingThreads.begin());
	queueOfRunningThreads.push_back(tempLIW);
	threadsRunning++;
	threadsWaiting--;

	// This format of call seems to have fixed the issues with access violation. Let's hope it
	// keeps on working.
	std::thread t1(&LongIntWrapper::startProcess, tempLIW);

	t1.detach();

	lk.unlock();
	}
	catch (std::exception e)
	{
		CString fred;
	}
}

UINT QueueOfThreads::numOfThreads()
{
	// For testing
	std::unique_lock<std::mutex> lk(lock);
	return threadsRunning + threadsWaiting;
	lk.unlock();
}

void QueueOfThreads::waitForAllToFinish()
{
	while (threadsWaiting > 0 || threadsRunning > 0)
	{
		// Put code in here to wait on the items in queueOfThreads to exit. I thought of using join(), but it says
		// it is inaccessible.
	}
}

void QueueOfThreads::receiveUpdate(UINT id)
{
	decreaseCount(id);
}

