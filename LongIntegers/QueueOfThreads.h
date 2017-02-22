#pragma once
#include <mutex>
#include <queue>
#include <vector>
#include <condition_variable>
#include "LongIntWrapper.h"
#include "ReceiveUpdateClass.h"

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
	std::mutex lock;
	std::vector<LongIntWrapper*> queueOfWaitingThreads;
	std::vector<LongIntWrapper*> queueOfRunningThreads;
	std::condition_variable cv;
	UINT threadID;

public:
	// Methods
	void decreaseCount(UINT id);
	bool addToQueue(LongIntWrapper*);
	void startAThread();
	void waitForAllToFinish();

	UINT numOfThreads();

	void receiveUpdate(UINT);
};

