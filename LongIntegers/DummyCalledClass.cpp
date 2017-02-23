#include "stdafx.h"
#include "DummyCalledClass.h"
#include "LongIntWrapper.h"


DummyCalledClass::DummyCalledClass()
{
}


DummyCalledClass::~DummyCalledClass()
{
	int i = 0;
}


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
