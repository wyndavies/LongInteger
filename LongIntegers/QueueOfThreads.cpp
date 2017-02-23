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
	unique_lock<mutex> lock(myMutex);
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
	lock.unlock();
	myConditionVariable.notify_one();

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
	
	unique_lock<mutex> lock(myMutex);

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
	lock.unlock();
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
	unique_lock<std::mutex> lock(myMutex);
	// The wait will unlock and then try to reacquire the lock when it wakes up
	myConditionVariable.wait(lock, [this]() { return threadsRunning < maxThreads; });


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

	lock.unlock();
	}
	catch (std::exception e)
	{
		CString fred;
	}
}

UINT QueueOfThreads::numOfThreads()
{
	// For testing
	unique_lock<mutex> lock(myMutex);
	return threadsRunning + threadsWaiting;
	lock.unlock();
}

void QueueOfThreads::waitForAllToFinish()
{
	while (threadsWaiting > 0 || threadsRunning > 0)
	{
		// Put code in here to wait on the items in queueOfThreads to exit. I thought of using join(), but it says
		// it is inaccessible.
	}
}

void QueueOfThreads::iHaveFinished(UINT id)
{
	decreaseCount(id);
}


void QueueOfThreads::iAmWaiting()
{
	if (threadsRunning == 0) abort(); // Test code

	// The current thread is waiting on something, so let the queue know that other threads can be started
	unique_lock<mutex> lock(myMutex);

	threadsRunning--;
	threadsWaiting++;

	// Although the lock will release when it goes out of scope, I need to unlock it before calling notify
	lock.unlock();
	myConditionVariable.notify_one();

}


void QueueOfThreads::iHaveStoppedWaiting()
{
	if (threadsWaiting == 0) abort(); // Test code

	unique_lock<mutex> lock(myMutex);
	myConditionVariable.wait(lock, [this]() { return threadsRunning < maxThreads; });

	threadsRunning++;
	threadsWaiting--;

	lock.unlock();
}