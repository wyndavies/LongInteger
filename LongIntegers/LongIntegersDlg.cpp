
// LongIntegersDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LongIntegers.h"
#include "LongIntegersDlg.h"
#include "afxdialogex.h"
#include <future>

#include <chrono>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CLongIntegersDlg dialog



CLongIntegersDlg::CLongIntegersDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CLongIntegersDlg::IDD, pParent)
{
	AfxInitRichEdit2();
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLongIntegersDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//  DDX_Control(pDX, IDC_OUTPUT, m_BigIntOutput);
	//  DDX_Control(pDX, IDC_EDIT2, m_ChangeNumber);
	DDX_Control(pDX, IDC_RICHEDIT1, m_RichEdit1);
	DDX_Control(pDX, IDC_RICHEDIT2, m_RichEdit2);
	DDX_Control(pDX, IDC_RICHEDIT3, m_OutputNumber);
	DDX_Control(pDX, IDC_RICHEDIT4, m_ArrowLevel);
}

BEGIN_MESSAGE_MAP(CLongIntegersDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDCANCEL, &CLongIntegersDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CLongIntegersDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDADD, &CLongIntegersDlg::OnClickedIdadd)
//	ON_EN_UPDATE(IDC_EDIT1, &CLongIntegersDlg::OnUpdateEdit1)
	ON_BN_CLICKED(IDMINUS, &CLongIntegersDlg::OnClickedIdminus)
	ON_BN_CLICKED(IDMULTIPLY, &CLongIntegersDlg::OnClickedIdmultiply)
	ON_BN_CLICKED(IDTOCLIP, &CLongIntegersDlg::OnClickedIdtoclip)
	ON_EN_UPDATE(IDC_RICHEDIT1, &CLongIntegersDlg::OnUpdateRichedit1)
	ON_EN_CHANGE(IDC_RICHEDIT1, &CLongIntegersDlg::OnChangeRichedit1)
	ON_BN_CLICKED(IDPOWER, &CLongIntegersDlg::OnClickedIdpower)
	ON_BN_CLICKED(IDARROW, &CLongIntegersDlg::OnClickedIdarrow)
	ON_BN_CLICKED(IDDIVIDE, &CLongIntegersDlg::OnClickedIddivide)
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()


// CLongIntegersDlg message handlers

BOOL CLongIntegersDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	int stuffsize = 1000000;
	m_RichEdit1.LimitText(stuffsize);
	m_RichEdit2.LimitText(stuffsize);
	m_OutputNumber.LimitText(stuffsize);

	m_longInt.setProcessing(nullptr, false);
	m_longInt.setShuttingDown(false);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CLongIntegersDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

BOOL CLongIntegersDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (m_longInt.isProcessing()) {
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_WAIT));
	}
	else {
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
	}
	// IDC_WAIT

	return TRUE;
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CLongIntegersDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CLongIntegersDlg::OnBnClickedCancel()
{
	// Add a bit of logic to make sure we don't shut down whilst background threads are still running
	// Not perfect as some functions can take a while (e.g. 2^^4 will take about 10 to the power 306 years)
	if (m_longInt.isProcessing()) {
		m_longInt.setShuttingDown(true);
	}
	while (m_longInt.isProcessing())
	{
		::WaitForSingleObject(m_longInt.getProcessingHandle(), INFINITE);
	}

	// This exits, so that is good. Don't have to create any special code.
	CDialogEx::OnCancel();
}


void CLongIntegersDlg::OnBnClickedOk()
{
	// This button triggers the typed in number in the first dialog to be validated
	// and then output in the main output window
	// Behind the scenes, the decimal number is converted into a base-256 number and stored in m_longInt

	// Get this to do the processing
	CString strValue;
	m_RichEdit1.GetWindowTextW(strValue);
	if (strValue == "") {
		return;
	}

	// Assign the value to m_longInt (and convert to base-256)
	m_longInt.assignNumber(strValue);

	// Display the number as a decimal
	m_OutputNumber.SetWindowTextW(m_longInt.toDecimal());
}


void CLongIntegersDlg::OnClickedIdadd()
{
	m_longInt.setProcessing(true);
	// Get the value from the 2nd box and add it to the main number
	CString strAddNumber;
	m_RichEdit2.GetWindowTextW(strAddNumber);

	LongInteger liAdd(strAddNumber);

	m_longInt += liAdd;

	m_OutputNumber.SetWindowTextW(m_longInt.toDecimal());
	m_longInt.setProcessing(false);
}

void CLongIntegersDlg::OnUpdateRichedit1()
{
	// If the input number has been changed, call the "OK" button function to update the main dialog
	OnBnClickedOk();
}


void CLongIntegersDlg::OnChangeRichedit1()
{
	// If the input number has been changed, call the "OK" button function to update the main dialog
	OnBnClickedOk();
}


void CLongIntegersDlg::OnClickedIdminus()
{
	// Get the value from the 2nd box and subtract it from the main number
	CString strMinusNumber;
	m_RichEdit2.GetWindowTextW(strMinusNumber);

	LongInteger liMinus(strMinusNumber);

	m_longInt -= liMinus;

	m_OutputNumber.SetWindowTextW(m_longInt.toDecimal());
}



void CLongIntegersDlg::OnClickedIdmultiply()
{
	if (!m_longInt.isProcessing()) {
		// Do the processing in the background
		m_longInt.setProcessing(true, AfxBeginThread(CLongIntegersDlg::StartMultWork, reinterpret_cast<LPVOID>(this)));
	}
}

UINT CLongIntegersDlg::StartMultWork(LPVOID param)
{
	// Call the function to perform multiplication.
	// We have been passed a pointer to the current dialog so we can update the dialog when we finish
	CLongIntegersDlg* thisDlg = reinterpret_cast<CLongIntegersDlg*>(param);

	CString strMultiplyNumber;
	thisDlg->m_RichEdit2.GetWindowTextW(strMultiplyNumber);

	LongInteger liMult(strMultiplyNumber);

	thisDlg->m_longInt *= liMult;

	CString strTemp = thisDlg->m_longInt.toDecimal();

	// If it is shutting down and SetWindowTextW is called everything freezes
	if (!thisDlg->m_longInt.isShuttingDown()) {
		thisDlg->m_OutputNumber.SetWindowTextW(strTemp);

		thisDlg->SendMessage(WM_PAINT);
	}
	thisDlg->m_longInt.setProcessing(false);

	return 0;
}



void CLongIntegersDlg::OnClickedIdtoclip()
{
	// Copy the output number to the clipboard
	// This somewhat convoluted code came from Microsoft's help files. And it works.
	// The clipboard holds unitext pointed to by a LPTSTR. A CString holds a LPTCSTR.
	// There are functions with obscure names to convert every type of string, so this isn't a problem
	CString strOut;
	m_OutputNumber.GetWindowTextW(strOut);

	HGLOBAL hGlob;
	LPTSTR lString;

	size_t datasize = (strOut.GetLength() + 1) * sizeof(TCHAR);
	hGlob = GlobalAlloc(GMEM_MOVEABLE, datasize);
	lString = (LPTSTR)GlobalLock(hGlob);

	ZeroMemory(lString, datasize);
	_tcscpy_s(lString, strOut.GetLength() + 1, strOut);
	strOut.ReleaseBuffer();
	GlobalUnlock(hGlob);

	OpenClipboard();
	EmptyClipboard();
	SetClipboardData(CF_UNICODETEXT, hGlob);
	CloseClipboard();
	//GlobalFree(hGlob); -- most people say this should be included, but it causes a resource leak in this application
}


void CLongIntegersDlg::OnClickedIddivide()
{
	if (!m_longInt.isProcessing())
	{
		m_longInt.setProcessing(true, AfxBeginThread(CLongIntegersDlg::StartDivWork, reinterpret_cast<LPVOID>(this)));
	}
}

UINT CLongIntegersDlg::StartDivWork(LPVOID param)
{
	CLongIntegersDlg* thisDlg = reinterpret_cast<CLongIntegersDlg*>(param);

	// Get the divisor
	CString strDivisor;
	thisDlg->m_RichEdit2.GetWindowTextW(strDivisor);

	LongInteger liDivide = strDivisor;

	thisDlg->m_longInt.divideNumber(liDivide);

	thisDlg->m_OutputNumber.SetWindowTextW(thisDlg->m_longInt.toDecimal());

	thisDlg->m_longInt.setProcessing(false);

	thisDlg->SendMessage(WM_PAINT);

	return 0;
}




void CLongIntegersDlg::OnClickedIdpower()
{
	if (!m_longInt.isProcessing()) {
		m_longInt.setProcessing(true, m_procthread = AfxBeginThread(CLongIntegersDlg::StartPowerWork, reinterpret_cast<LPVOID>(this)));
	}
}

UINT CLongIntegersDlg::StartPowerWork(LPVOID param)
{
	CLongIntegersDlg* thisDlg = reinterpret_cast<CLongIntegersDlg*>(param);

	CString strOut;
	thisDlg->m_RichEdit2.GetWindowTextW(strOut);

	LongInteger liMult = strOut;

	// Start by just calling the IdPower method
	thisDlg->m_longInt.powerCalc(thisDlg->m_longInt, liMult);

	if (!thisDlg->m_longInt.isShuttingDown()) {
		thisDlg->m_OutputNumber.SetWindowTextW(thisDlg->m_longInt.toDecimal());
		thisDlg->SendMessage(WM_PAINT);
	}
	thisDlg->m_longInt.setProcessing(false);
	thisDlg = nullptr;
	return 0;
}


void CLongIntegersDlg::OnClickedIdarrow()
{
	// For this we do something different. The initial number is ignored
	// It will be 2(n arrows)2, with 'n' coming from the lower input
	// Arrow notation is 'big' powers.
	// 1 arrow gives 2^2
	// 2 arrow gives a tower the height of the 2nd number (which comes out as 2^2)
	// 3 arrow gives 2^(2||2)->2^2^2^2. A tower 2||2 high. This comes out as 65536.
	// 4 arrow gives 2^(2|||2). Which is a tower 2|||2 (or 2^2^2^2) high. No-one has calculated this yet as it is so large.
	// 5 arrow gives 2^(2||||2). Which is a tower 2||||2 high
	// So each step involves working out the one below.
	// 2||||2 -> 2|(2|||2) -> 2|(2|(2||2)) -> 2|(2|(2|(2|2)))


	// Tests for random stuff under the "Arrow" button, because why not

	// Test the new division algorithms
	LongInteger liNumber, liDivisor, *pliQuotient, *pliModulus;

	for (UINT i = 10; i < 100; i++)
	{


		byte *array1 = new byte[i];
		for (UINT digits = 0; digits < i; digits++)
		{
			array1[digits] = 10;
		}
		liNumber.assignByteArray(array1, i);
		for (UINT j = 6; j < i; j++)
		{
			byte *array2 = new byte[j];
			for (UINT digits = 0; digits < j; digits++)
			{
				array2[digits] = 5;
			}

			liDivisor.assignByteArray(array2, j);

			// Test Burnikel-Ziegler division
			pliModulus = new LongInteger; // Make these shared pointers later on to simplify the memory management headaches
			pliQuotient = new LongInteger;
			LongInteger::BurnikelZiegler(liNumber, liDivisor, pliQuotient, pliModulus);

			LongInteger liRealQuotient = liNumber / liDivisor;
			LongInteger liRealModulus = liNumber % liDivisor;

			CString strTestRQ, strTestRM, strTestQ, strTestM;
			strTestRQ = liRealQuotient.toArrayNumbers();
			strTestRM = liRealModulus.toArrayNumbers();
			strTestQ = pliQuotient->toArrayNumbers();
			strTestM = pliModulus->toArrayNumbers();

			bool bWorked1 = *pliQuotient == liRealQuotient;
			bool bWorked2 = *pliModulus == liRealModulus;
			if (!(bWorked1 && bWorked2))
				int iBreakPointInt = 0;

			delete pliModulus;
			delete pliQuotient;
			delete array2;
		}
		delete array1;
	}
	return;

	// Test restoring division
	liNumber = 25;
	liDivisor = 10;

	LongInteger::RestoringDivision(liNumber, liDivisor, pliQuotient, pliModulus);
	CString divResult;
	divResult.Format(L"%s divided by %s = %s, remainder = %s", liNumber.toDecimal(), liDivisor.toDecimal(), pliQuotient->toDecimal(), pliModulus->toDecimal());


	liNumber = 10000;
	liDivisor = 111;
	LongInteger::RestoringDivision(liNumber, liDivisor, pliQuotient, pliModulus);
	// Result = 90, remainder 10
	divResult.Format(L"%s divided by %s = %s, remainder = %s", liNumber.toDecimal(), liDivisor.toDecimal(), pliQuotient->toDecimal(), pliModulus->toDecimal());

	liNumber = CString(L"1000000000000000");
	liDivisor = 111111;
	LongInteger::RestoringDivision(liNumber, liDivisor, pliQuotient, pliModulus);
	// Result = 900090009000, remainder 1000
	divResult.Format(L"%s divided by %s = %s, remainder = %s", liNumber.toDecimal(), liDivisor.toDecimal(), pliQuotient->toDecimal(), pliModulus->toDecimal());


	delete pliQuotient;
	delete pliModulus;












	// Performance and bug testing for multiplication algorithm
	CString strNum1, strNum2, strBill, strBob;
	LongInteger bob, num1, num2, frank;
	LongInteger *bill;
	bool bResult, bResult2;

	// Some numbers to test ToomCook3
	strNum1 = L"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
	strNum2 = L"9876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210";

	CStdioFile writeHex;
	writeHex.Open(L"D:\\hextex.txt", CFile::modeCreate | CFile::modeReadWrite);
	CString newline = L"\n";
	CString writing;
	auto startTimetc3 = std::chrono::high_resolution_clock::now();
	auto endTimetc3 = std::chrono::high_resolution_clock::now();
	auto startTimek = std::chrono::high_resolution_clock::now();
	auto endTimek = std::chrono::high_resolution_clock::now();
	auto startTimem = std::chrono::high_resolution_clock::now();
	auto endTimem = std::chrono::high_resolution_clock::now();


	// Create a byte array for testing big numbers
	/*for (UINT arraysize = 1000; arraysize < 10000; arraysize+=1000) {
		for (UINT cutoff = 100; cutoff > 10; cutoff-=10) {
			UINT bytearraysize = arraysize;
			byte* newArray1 = new byte[arraysize];
			byte* newArray2 = new byte[arraysize];
			memset(newArray1, 100, bytearraysize);
			memset(newArray2, 200, bytearraysize);


			LongInteger num1(0);
			LongInteger num2(0);
			num1.assignByteArray(newArray1, bytearraysize);
			num2.assignByteArray(newArray2, bytearraysize);

			LongInteger* result1 = LongInteger::ToomCook3(num1, num2, cutoff);
			LongInteger result2 = num1 * num2;
			LongInteger result3 = num1;
			result3.multiplyInternal(num2);

			CString strResult1 = result1->toHexString();
			CString strResult2 = result2.toHexString();



			bool equal1 = *result1 == result2;
			if (!equal1) {
				LongInteger difference = *result1 - result2;
				CString strDiff = difference.toDecimal();
				/*-----------------------------------*/
				/*writeHex.Open(L"D:\\hextex.txt", CFile::modeReadWrite | CFile::modeNoTruncate);
				writeHex.SeekToEnd();
				CString hexout;
				hexout.Format(L"{%d}", arraysize);
				writeHex.WriteString(hexout);
				writeHex.WriteString(strResult1);
				writeHex.WriteString(newline);
				writeHex.WriteString(strResult2);
				writeHex.WriteString(newline);
				writeHex.WriteString(newline);
				writeHex.Close();*/
				/*-----------------------------------*/
/*			}
			bool equal2 = *result1 == result3;
			bool equal3 = result2 == result3;

			bool allequal = equal1 & equal2 & equal3;

			if (!allequal)
				int breakpointint = 0;

			delete result1;
			delete newArray1;
			delete newArray2;
		}
	}*/
	UINT bytearraysize = 100000;
	byte* newArray1;
	byte* newArray2;

	UINT size = bytearraysize;
	UINT step = size / 40;

	

	for (UINT i = 1; i < 2000; i++) {
		for (UINT j = 1; j < 5; j++) {
			bytearraysize = i;
			newArray1 = new byte[bytearraysize];
			newArray2 = new byte[bytearraysize];
			memset(newArray1, 100, bytearraysize);
			memset(newArray2, 200, bytearraysize);
			num1.assignByteArray(newArray1, bytearraysize);
			num2.assignByteArray(newArray2, bytearraysize);
			startTimetc3 = std::chrono::high_resolution_clock::now();
			bill = LongInteger::ToomCook3(num1, num2);
			endTimetc3 = std::chrono::high_resolution_clock::now();
			startTimek = std::chrono::high_resolution_clock::now();
			bob = (num1 * num2);
			endTimek = std::chrono::high_resolution_clock::now();

			frank = num1;
			startTimem = std::chrono::high_resolution_clock::now();
			frank.multiplyInternal(num2);
			endTimem = std::chrono::high_resolution_clock::now();

			bResult = *bill == bob;
			bResult2 = *bill == frank;

			if (!(bResult & bResult2))
				int breakpointint = 0;

			auto diff1 = std::chrono::duration_cast<std::chrono::microseconds>(endTimetc3 - startTimetc3).count();
			auto diff2 = std::chrono::duration_cast<std::chrono::microseconds>(endTimek - startTimek).count();
			auto diff3 = std::chrono::duration_cast<std::chrono::microseconds>(endTimem - startTimem).count();
			writing.Format(L"Size,%d,TC3,%d,Karatsuba,%d,Multiplication,%d,", i, diff1, diff2, diff3);
			if (diff1 < diff2 && diff1 < diff3) writing.Append(L"TC3");
			if (diff2 < diff1 && diff2 < diff3) writing.Append(L"Kara");
			if (diff3 < diff1 && diff3 < diff2) writing.Append(L"M");
			if (diff1 == diff2 && diff1 < diff3) writing.Append(L"TC3 & K");
			if (diff1 == diff3 && diff1 < diff2) writing.Append(L"TC3 & M");
			if (diff2 == diff3 && diff2 < diff1) writing.Append(L"K & M");
			if (diff1 == diff2 && diff1 == diff3) writing.Append(L"TC3 & K & M");

			writeHex.WriteString(writing);
			writeHex.WriteString(newline);
			delete bill;
			delete newArray1;
			delete newArray2;
		}
	}
	writeHex.Close();

	CString strRE2;
	m_RichEdit2.GetWindowTextW(strRE2);
	LongInteger liInput = strRE2;

	LongInteger liPos1(10);
	LongInteger liNeg1(-10);
	LongInteger liPos2(CString("20"));
	LongInteger liNeg2(CString("-20"));

	LongInteger liTest;

	bResult = ((liTest = liPos1 + liNeg1) == 0); // 0
	bResult = ((liTest = liPos2 + liNeg2) == 0); // 0
	bResult = ((liTest = liPos1 + liNeg2) == -10); // -10
	bResult = ((liTest = liPos2 + liNeg1) == 10); // +10
	bResult = ((liTest = liNeg1 + liPos1) == 0); // 0
	bResult = ((liTest = liNeg1 + liNeg2) == -30); // -30
	bResult = ((liTest = liNeg1 + liPos2) == 10); // +10
	bResult = ((liTest = liNeg2 + liPos1) == -10); // -10
	liTest = liPos1;
	bResult = ((liTest += liNeg1) == 0); // 0
	liTest = liPos1;
	bResult = ((liTest += liNeg2) == -10); // -10
	liTest = liNeg1;
	bResult = ((liTest += liPos1) == 0); // 0
	liTest = liNeg1;
	bResult = ((liTest += liPos2) == +10); // +10
	liTest = liNeg1;
	bResult = ((liTest += liNeg2) == -30); // -30
	liTest = liNeg2;
	bResult = ((liTest += liPos1) == -10); // -10

	// Now test subtractions
	liTest = liPos2 - liPos1; // +10
	liTest = liPos1 - liPos2; // -10
	liTest = liPos1 - liNeg1; // +20
	liTest = liNeg1 - liPos1; // -20
	liTest = liNeg1 - liNeg2; // +10 (-10 - -20 => -10 + 20 => +10)

	// Test additions and subtracts with ints
	liTest = 1 + liNeg1; // -9
	liTest = liPos1 - 20; // -10
	liTest = liPos1 + (-20); // -10
	liTest = liNeg1 + 20; // +10
	liTest = liNeg1 - 10; // -20
	liTest = (-10) + liPos2; // +10
	liTest = (-10) - liNeg1; // 0
	liTest = (-10) + liNeg1; // -20
	liTest = (-10) - liPos2; // -30

	// Now test comparisons
	bResult = (liPos1 == liNeg1); // false
	bResult = (liNeg1 == liNeg1); // true
	bResult = (liPos1 == (-liNeg1)); // true
	bResult = ((-liNeg1) == liPos1); // true
	bResult = ((-liNeg2) == liPos1); // false
	bResult = (liPos1 > liNeg1); // true
	bResult = (liPos1 > liNeg2); // true
	bResult = (liNeg1 > liPos1); // false
	bResult = (liNeg1 > liNeg2); // true
	bResult = (liPos1 < liNeg1); // false
	bResult = (liPos1 < liNeg2); // false
	bResult = (liNeg1 < liPos1); // true
	bResult = (liNeg1 < liNeg2); // false
	bResult = (liPos1 != liNeg1); // true
	bResult = (liNeg1 != liPos1); // true
	bResult = (liNeg1 != liNeg1); // false

	liTest = liPos1;
	liTest -= 9; // 1
	liTest--; // 0
	liTest--; // -1
	liTest--; // -2
	liTest++; // -1
	liTest++; // 0
	liTest++; // 1

	// Now check multipling involving negative numbers
	bResult = ((liTest = liPos1 * liPos2) == 200);
	bResult = ((liTest = liPos1 * liNeg1) == -100);
	bResult = ((liTest = liNeg1 * liPos1) == -100);
	bResult = ((liTest = liNeg1 * liNeg2) == 200);

	// Now check multiplying zero
	bResult = ((liTest = 0 * liPos1) == 0);
	bResult = ((liTest = 0 * liNeg1) == 0);
	bResult = ((liTest = liPos1 * 0) == 0);
	bResult = ((liTest = liNeg1 * 0) == 0);

	// Now for division
	bResult = ((liTest = liPos2 / liPos1) == 2);
	bResult = ((liTest = liNeg2 / liPos1) == -2);
	bResult = ((liTest = liPos2 / liNeg1) == -2);
	bResult = ((liTest = liNeg2 / liNeg1) == 2);

	// Now for division
	bResult = ((liTest = liPos2 / 0) == 0); // bOverflow = true
	bResult = ((liTest = liNeg2 / 0) == 0); // bOverflow = true
	bResult = ((liTest = 0 / liNeg1) == 0);
	bResult = ((liTest = 0 / liPos1) == 0);


	LongInteger liZero(0);
	LongInteger liN(88);
	LongInteger liResult;
	bResult = false;
	// What haven't we tested yet?
	// Zeroes.
	// Add a zero
	bResult = (88 == (liN + liZero));
	// Add to zero
	bResult &= (88 == (liZero + liN));
	// Add two zeroes
	bResult &= (0 == (liZero + liZero));
	// Subtract a zero
	bResult &= (88 == (liN - liZero));
	// Subtract from zero
	bResult &= !(88 == (liZero - liN));
	// Subtract two zeroes
	bResult &= (0 == (liZero - liZero));
	// Multiply by zero
	bResult &= (0 == (liN * liZero));
	// Multiply zero
	bResult &= (0 == (liZero * liN));
	// Multiply 0*0
	bResult &= (0 == (liZero * liZero));
	// Divide by zero
	bResult &= (88 == (liN / liZero)); // Divide by zero is refused and the overflow flag is set
	// divide zero
	bResult &= (0 == (liZero / liN));
	// divide 0/0
	bResult &= (0 == (liZero / liZero));
	// Power of zero
	bResult &= (liResult = liN, liResult.powerCalc(liZero), 1 == liResult);
	// 0^n
	bResult &= (liResult = liZero, liResult.powerCalc(liN), 0 == liResult);
	// 0^0
	bResult &= (liResult = liZero, liResult.powerCalc(liZero), 1 == liResult);
	// Rightshift by zero
	bResult &= (0 == (liZero >> liN));
	// Rightshift zero
	bResult &= (88 == (liN >> liZero));
	// Rightshift zero by zero
	bResult &= (0 == (liZero >> liZero));
	// Leftshift by zero
	bResult &= (0 == (liZero << liN));
	// Leftshift zero
	bResult &= (88 == (liN << liZero));
	// Leftshift zero by zero
	bResult &= (0 == (liZero << liZero));
	// not zero
	bResult &= (0 == ~liZero);
	// n OR zero
	bResult &= (88 == (liN | liZero));
	// zero OR n
	bResult &= (0 == (liZero | liN)); // liZero is zero bytes in length before and after - so the final value is 0
	// zero OR zero
	bResult &= (0 == (liZero | liZero));
	// n XOR zero
	bResult &= (88 == (liN ^ liZero));
	// zero XOR n
	bResult &= (0 == (liZero ^ liN)); // liZero is zero bytes in length before and after - so the final value is 0
	// zero XOR zero
	bResult &= (0 == (liZero ^ liZero));
	// n AND zero
	bResult &= (88 == (liN & liZero)); // liN won't change as liZero is zero bytes in length
	// zero AND n
	bResult &= (0 == (liZero & liN));
	// zero AND zero
	bResult &= (0 == (liZero & liZero));
	// is n == zero
	bResult &= !(liN == liZero);
	// is zero == n
	bResult &= !(liZero == liN);
	// is zero == zero
	bResult &= (liZero == liZero);
	// is n != zero
	bResult &= (liN != liZero);
	// is zero != n
	bResult &= (liZero != liN);
	// is zero != zero
	bResult &= !(liZero != liZero);
	// is n > zero
	bResult &= (liN > liZero);
	// is n >= zero
	bResult &= (liN >= liZero);
	// is n < zero
	bResult &= !(liN < liZero);
	// is n <= zero
	bResult &= !(liN <= liZero);
	// is zero > n
	bResult &= !(liZero > liN);
	// is zero >= n
	bResult &= !(liZero >= liN);
	// is zero < n
	bResult &= (liZero < liN);
	// is zero <= n
	bResult &= (liZero <= liN);
	// is zero > zero
	bResult &= !(liZero > liZero);
	// is zero >= zero
	bResult &= (liZero >= liZero);
	// is zero < zero
	bResult &= !(liZero < liZero);
	// is zero <= zero
	bResult &= (liZero <= liZero);


	m_OutputNumber.SetWindowTextW(m_longInt.toDecimal());

	/*
	CString strFactor;
	CString strArrowLevel;
	UINT iFactor;
	int iArrowLevel;

	m_RichEdit2.GetWindowTextW(strFactor);
	m_ArrowLevel.GetWindowTextW(strArrowLevel);
	iFactor = _ttoi(strFactor);
	iArrowLevel = _ttoi(strArrowLevel);

	m_longInt.arrowCalc(iArrowLevel, iFactor);

	m_OutputNumber.SetWindowTextW(m_longInt.toDecimal());
	*/
}
