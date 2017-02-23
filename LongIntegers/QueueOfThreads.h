#pragma once
#include <mutex>
#include <queue>
#include <vector>
#include <condition_variable>
#include "LongIntWrapper.h"
#include "ReceiveUpdateClass.h"

using std::vector;
using std::mutex;
using std::unique_lock;
using std::condition_variable;

class LongIntWrapper;

class QueueOfThreads : ReceiveUpdateClass
{
public:
	QueueOfThreads();
	~QueueOfThreads();
private:
	// Attributes
	UINT threadsRunning;
	UINT threadsWaiting;
	UINT maxThreads;
	static const UINT minThreads = 4;
	mutex myMutex;
	vector<LongIntWrapper*> queueOfWaitingThreads;
	vector<LongIntWrapper*> queueOfRunningThreads;
	condition_variable myConditionVariable;
	UINT threadID;

public:
	// Methods
	void decreaseCount(UINT id);
	bool addToQueue(LongIntWrapper*);
	void startAThread();
	void waitForAllToFinish();

	UINT numOfThreads();

	// Tell the queue that the current thread has finished
	void iHaveFinished(UINT id);

	// Tell the queue that the current thread is waiting and so remove it from the count of running threads
	void iAmWaiting();
	// Tell the queue that the current thread is ready to start running again
	void iHaveStoppedWaiting();

	// Wait for a thread to finish
	void waitForThread(LongIntWrapper*);
};

