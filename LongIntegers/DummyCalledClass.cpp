#include "stdafx.h"
#include "DummyCalledClass.h"


DummyCalledClass::DummyCalledClass()
{
}


DummyCalledClass::~DummyCalledClass()
{
}


CString DummyCalledClass::createAFile(int param1, int param2)
{
	CString fileName;
	if (param1 < 1) param1 = 1;
	if (param2 < 1) param2 = 1;
	fileName.Format(L"D:\\ZZZ\\%d%d.txt", param1, param2);

	CStdioFile myFile;
	bool bSuccess = myFile.Open(fileName, CFile::modeCreate | CFile::modeWrite);
	myFile.WriteString(L"Hello");
	myFile.Close();

	return fileName;
}
