/*
	DummyCalledClass.cpp
	Created for testing. This just creates a file with a number for a name. The number gives info on the process calling
	it. As the numbers come in as 2 parameters, the caller can populate them with whatever numbers they like


*/


#ifdef _WIN32
#include "stdafx.h"
#endif

#include "DummyCalledClass.h"
#include "LongIntWrapper.h"

#ifndef _WIN32
// File and StringStream classes for non-windows platforms
#include <sstream>
#include <fstream>
#endif

DummyCalledClass::DummyCalledClass()
{
}


DummyCalledClass::~DummyCalledClass()
{
	int i = 0;
}


#ifdef _WIN32
// Windows version of writing a file to disk
CString DummyCalledClass::createAFile(int param1, int param2)
{
	CString fileName;

	fileName.Format(L"D:\\ZZZ\\%d%d.txt", param1, param2);
	CStdioFile myFile;
	bool bSuccess = myFile.Open(fileName, CFile::modeCreate | CFile::modeWrite);
	myFile.WriteString(L"Hello");
	myFile.Close();

	return fileName;

}
#else

string DummyCalledClass::createAFile(int param1, int param2)
{
	std::ostringstream tempstr;
	string fileName;

	tempstr << "~/" << param1 << param2 << ".txt";
	fileName = tempstr.str();

	std::ofstream myFile;
	myFile.open(fileName);
	myFile << "Hello\n";
	myFile.close();

	return fileName;
}

#endif

#ifdef _WIN32
CString DummyCalledClass::recursiveCall(int param1, int param2)
{
	// This is the recursive section
	// However it does not call 'createAFile' directly
	// It will call createAFile to create a file with the parameters given,
	// then it will count down param2 until it reaches zero, calling itself
	// via the threading class

	createAFile(param1, param2);
	if (param2 > 0)
	{
		param2--;
		LongIntWrapper* pLIW = new LongIntWrapper();
		pLIW->setDummyParams(param1, param2);

		LongIntWrapper::getQOT()->addToQueue(pLIW);

		// Wait for it to finish
		// This thread is now in a waiting state. Need to amend the count in qot
		// otherwise we will have a deadlock
		pLIW->getQOT()->iAmWaiting();
		while (!(pLIW->bFinished))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		pLIW->getQOT()->iHaveStoppedWaiting();
		delete pLIW;
	}


	CString returnValue;
	return returnValue;
}
#else

string DummyCalledClass::recursiveCall(int param1, int param2)
{
	// This is the recursive section
	// However it does not call 'createAFile' directly
	// It will call createAFile to create a file with the parameters given,
	// then it will count down param2 until it reaches zero, calling itself
	// via the threading class

	createAFile(param1, param2);
	if (param2 > 0)
	{
		param2--;
		LongIntWrapper* pLIW = new LongIntWrapper();
		pLIW->setDummyParams(param1, param2);

		LongIntWrapper::getQOT()->addToQueue(pLIW);

		// Wait for it to finish
		// This thread is now in a waiting state. Need to amend the count in qot
		// otherwise we will have a deadlock
		pLIW->getQOT()->iAmWaiting();
		while (!(pLIW->bFinished))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		pLIW->getQOT()->iHaveStoppedWaiting();
		delete pLIW;
	}


	string returnValue;
	return returnValue;
}
#endif