/*
GeneralIntWrapper.h

This class is a wrapper to LongInteger although it can also wrap other items. It allows LongIntegers to be sent
to QueueOfThreads and then collects the results of processing to be picked up by the caller.
It has been created to separate LongInteger and QueueOfThreads

At the moment it just calls the karatsuba algorithm. Will need to be amended to allow for calling any mr.
*/

#pragma once

#ifndef _WIN32
typedef unsigned int UINT;
#endif

#include "QueueOfThreads.h"



template<class T> class GeneralIntWrapper
{
public:
	GeneralIntWrapper();
	~GeneralIntWrapper();

	typedef T* (*Tfunction)(const T&, const T&, bool);

private:
	UINT id;
	T* param1;
	T* param2;
	T* pResult;
	ReceiveUpdateClass<GeneralIntWrapper<T>> *callbackObject;
	Tfunction startMethod;
	static QueueOfThreads<GeneralIntWrapper<T>> qot;

public:
	bool bRunning;
	bool bFinished;
	int startProcess();
	void setCallback(ReceiveUpdateClass<GeneralIntWrapper<T>>*);
	void setStartMethod(Tfunction newMethod);
	void setID(UINT);
	UINT getID();
	static QueueOfThreads<GeneralIntWrapper<T>>* getQOT();

	T* getResult();
	void setParams(const T& rParam1, const T& rParam2);
};


#include "GeneralIntWrapper.hpp"