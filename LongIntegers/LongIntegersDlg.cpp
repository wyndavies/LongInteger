
// LongIntegersDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LongIntegers.h"
#include "LongIntegersDlg.h"
#include "afxdialogex.h"
#include <future>

#include <chrono>

#include "PrimeSwing.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



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

	m_longInt.setProcessing(false);
//	m_longInt.setShuttingDown(false);

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
		Sleep(100);
//		::WaitForSingleObject(m_longInt.getProcessingHandle(), INFINITE);
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
		m_longInt.setProcessing(true);
		AfxBeginThread(CLongIntegersDlg::StartMultWork, reinterpret_cast<LPVOID>(this));
//		m_longInt.setProcessing(true, AfxBeginThread(CLongIntegersDlg::StartMultWork, reinterpret_cast<LPVOID>(this)));
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
		m_longInt.setProcessing(true);
		AfxBeginThread(CLongIntegersDlg::StartDivWork, reinterpret_cast<LPVOID>(this));
//		m_longInt.setProcessing(true, AfxBeginThread(CLongIntegersDlg::StartDivWork, reinterpret_cast<LPVOID>(this)));
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
		m_longInt.setProcessing(true);
		AfxBeginThread(CLongIntegersDlg::StartPowerWork, reinterpret_cast<LPVOID>(this));
//		m_longInt.setProcessing(true, m_procthread = AfxBeginThread(CLongIntegersDlg::StartPowerWork, reinterpret_cast<LPVOID>(this)));
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

CString Divit(LongInteger& liBigNumber)
{
	CString strResult;
	UINT digits = 1;

	LongInteger* liWorking = new LongInteger(1);

	if (liWorking->getSize() < (liBigNumber.getSize() / 2) && liBigNumber.getSize() > 2) {
		// Try a different method
		// Work out the power and divide repeatedly by 2
		// Keep a running total of the base value multiplying by itself each loop
		// When a division by 2 gives an odd number, multiply the running total of the base values to another running total

		vector<bool> bList;
		UINT power = 1;
		if (liBigNumber.getSize() > 100) {
			power = liBigNumber.getSize();
			power += power;
		}
		else {
			power = liBigNumber.getSize();
		}
		digits = power;
		// Now what are the powers of 2 of this number?
		UINT n = 0;
		while (power > 0) {
			UINT temp = power / 2;
			if (temp * 2 != power) {
				bList.push_back(true);
			} 
			else {
				bList.push_back(false);
			}
			power = temp;
			n++;
		}

		LongInteger liTotal = 1;
		LongInteger liTen = 10;
		for (UINT j = 0; j < n; j++)
		{
			if (bList[j]) {
				liTotal *= liTen;
			}
			liTen *= liTen;
		}

		*liWorking = liTotal;

	}
	else
	{
		*liWorking = 10;
	}

	LongIntegerUP upliRes = make_unique<LongInteger>(0);
	LongIntegerUP upliMod = make_unique<LongInteger>(0);

	LongInteger::DivAndMod(liBigNumber, *liWorking, upliRes, upliMod);


	if (upliMod->getSize() > 1) {
		strResult = Divit(*upliMod);
		while ((UINT)strResult.GetLength() < digits) {
			strResult = L"0" + strResult;
		}
	}
	else {
		strResult = upliMod->toDecimal();
		while ((UINT)strResult.GetLength() < digits) {
			strResult = L"0" + strResult;
		}
	}

	if (upliRes->getSize() > 1) {
		strResult = Divit(*upliRes) + strResult;
	}
	else {
		strResult = upliRes->toDecimal() + strResult;
	}

	delete liWorking;
	return strResult;
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

	LongInteger* whatdahell = new LongInteger;
	CString filename = L"D:\\factorial.txt";
	whatdahell->readFromFile(filename);

	LongInteger* liAdd = new LongInteger;
	LongInteger* liMinus = new LongInteger;
	LongInteger* liMultiply = new LongInteger;
	LongInteger* liDivide = new LongInteger;


	*liDivide = *whatdahell / LongInteger(CString(L"1234567890123456789012345678901234567890"));

	*liAdd = *whatdahell + *whatdahell;
	*liMinus = *liAdd - *whatdahell;
	*liMultiply = *whatdahell * *whatdahell;
	*liDivide = *liMultiply / *whatdahell;
	delete liAdd;
	delete liMinus;
	delete liMultiply;
	delete liDivide;
	delete whatdahell;


/*	LongInteger* whatdahell;
	CStdioFile whatFile;
	BOOL bSuccess = whatFile.Open(L"D:\\number.txt", CFile::modeRead);
	CString* whatNumber = new CString;
	bSuccess = whatFile.ReadString(*whatNumber);
	whatFile.Close();
	whatdahell = new LongInteger(*whatNumber);
	LongInteger *wdh1, *wdh2, *wdh3, *wdh4;
	wdh1 = new LongInteger();
	wdh2 = new LongInteger();
	wdh3 = new LongInteger();
	wdh4 = new LongInteger();

	*wdh1 = *whatdahell + *whatdahell;
	*wdh2 = *wdh1 - *whatdahell;
	*wdh3 = *whatdahell * *whatdahell;
	*wdh4 = *wdh3 / *whatdahell;

	delete whatNumber;
	delete whatdahell;
	delete wdh1;
	delete wdh2;
	delete wdh3;
	delete wdh4;
	*/

	
	LongInteger five, twentyfour, quotient, modulus;

	five = 5;
	twentyfour = 24;
	LongInteger::DivAndMod(five, twentyfour, quotient, modulus);
	CString quot = quotient.toDecimal();
	CString modu = modulus.toDecimal();



/*	LongInteger unsquared = CString(L"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890");
	LongInteger squared = LongInteger::sqr(unsquared);
	LongInteger squareroot = LongInteger::sqrt(squared);
	bool squareworks = unsquared == squareroot;
	squared--;
	squareroot = LongInteger::sqrt(squared);
	squareworks = (unsquared - 1) == squareroot;
	*/


	PrimeSwing ps;
	LongInteger liPSResult;
	LongInteger liPSResultli;
	LongInteger liFactorial;
	CString psResult;
	bool psWorked, psWorked2;
	CString psAnswer[] = { L"1", L"1", L"2", L"6", L"24", L"120", L"720", L"5040", L"40320", L"362880", L"3628800",
		L"39916800", L"479001600", L"6227020800", L"87178291200", L"1307674368000", L"20922789888000", L"355687428096000", L"6402373705728000", L"121645100408832000", L"2432902008176640000",
		L"51090942171709440000", L"1124000727777607680000", L"25852016738884976640000", L"620448401733239439360000", L"15511210043330985984000000", L"403291461126605635584000000", L"10888869450418352160768000000", L"304888344611713860501504000000", L"8841761993739701954543616000000", L"265252859812191058636308480000000",
		L"8222838654177922817725562880000000", L"263130836933693530167218012160000000", L"8683317618811886495518194401280000000", L"295232799039604140847618609643520000000", L"10333147966386144929666651337523200000000", L"371993326789901217467999448150835200000000", L"13763753091226345046315979581580902400000000", L"523022617466601111760007224100074291200000000", L"20397882081197443358640281739902897356800000000", L"815915283247897734345611269596115894272000000000"
	};

	for (int i = 1; i < 41; i++)
	{
		liPSResult = ps.Factorial(i);
		liPSResultli = ps.Factorial(LongInteger(i));
		psResult = liPSResult.toDecimal();

		psWorked = psResult == psAnswer[i];
		psWorked2 = liPSResultli == liPSResult;
		if (!(psWorked && psWorked2))
		{
			CString failed = L"failed";
			CString psAns = psAnswer[i];
			CString difference = (LongInteger(psAns) / LongInteger(psResult)).toDecimal();
		}
		liFactorial = LongInteger::factorial(i);
		psWorked = liPSResult == liFactorial;
		if (!psWorked)
		{
			CString failed = L"failed";
		}
	}


/*	for (int i = 1000; i < 10001; i+=1000)
	{
		liPSResult = i;
		liPSResult.factorial();
		liFactorial = LongInteger::factorial(i);

		psWorked = liPSResult == liFactorial;
		if (!(psWorked))
		{
			CString failed = L"failed";
			CString oldStyleFactorial = liPSResult.toDecimal();
			CString internalCalced = liFactorial.toDecimal();
		}
	}
*/


//	liFactorial = LongInteger::factorial(1000000);

//	liFactorial.writeToFile(CString(L"D:\\factorial.txt"));
	
	//m_OutputNumber.SetWindowTextW(liFactorial.toDecimal());

	return;

	LongInteger::KARATSUBACUTOFF = 50;
	LongInteger::TOOMCOOK3CUTOFF = 100;

	// Test the factorial function
	LongInteger liFactorialMax = 100;
	LongInteger liResult;
	CString result;
	for (LongInteger i = 0; i < liFactorialMax; i++)
	{
		liResult = i;
		liResult.factorial();
		result = liResult.toDecimal();
	}


	return;


	// Some tests for multiplication
	LongInteger liVal1, liVal2, liResult1, liResult2, liResult3;


	CStdioFile myFile;
	//BOOL bSuccess = myFile.Open(L"D:\\result.txt", CFile::modeCreate | CFile::modeWrite);

	// Test the different types of multiplication.
	// Although the Karatsuba algorithm is part of multiplication, we can effectively disable it by setting the
	// cutoff to a value greater than the number of digits in the numbers involved
	
	for (UINT i = 100; i < 10000; i += 100)
	{
		// The size of the numbers being multiplied is i

		// Create the numbers
		byte* tempArray = new byte[i];
		for (UINT z = 0; z < i; z++)
		{
			tempArray[z] = 100;
		}
		liVal1.assignByteArray(tempArray, i);
		liVal2.assignByteArray(tempArray, i); // This is fine as the method copies the byte array
		delete tempArray;

		for (UINT j = 5; j < (i / 10); j+=5)
		{
			// j is the cutoff point for the algorithms

			// First we do long multiplication
			LongInteger::KARATSUBACUTOFF = 1000000;

			auto divStart = std::chrono::high_resolution_clock::now();

			liResult1 = liVal1 * liVal2;

			auto divEnd = std::chrono::high_resolution_clock::now();
			auto rawduration = divEnd - divStart;
			std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(rawduration);
			CString writeString;
			writeString.Format(L"%d,%d,Long,%d,", i, j, duration);
			myFile.WriteString(writeString);

			// Now for Karatsuba
			LongInteger::KARATSUBACUTOFF = j;
			divStart = std::chrono::high_resolution_clock::now();
			liResult2 = liVal1 * liVal2;
			divEnd = std::chrono::high_resolution_clock::now();
			rawduration = divEnd - divStart;
			duration = std::chrono::duration_cast<std::chrono::milliseconds>(rawduration);
			writeString.Format(L"%d,%d,Karatsuba,%d,", i, j, duration);
			myFile.WriteString(writeString);

			// Now for ToomCook3
			LongInteger::KARATSUBACUTOFF = 1000000; // Disable it to simplify matters
			LongInteger::TOOMCOOK3CUTOFF = j;
			divStart = std::chrono::high_resolution_clock::now();
			liResult3 = LongInteger::ToomCook3(liVal1, liVal2);
			divEnd = std::chrono::high_resolution_clock::now();
			rawduration = divEnd - divStart;
			duration = std::chrono::duration_cast<std::chrono::milliseconds>(rawduration);

			// Check that everything is all happiness and light
			bool bWorked;
			bWorked = liResult1 == liResult2;
			bWorked &= (liResult1 == liResult3);
			if (!bWorked)
			{
				int debugpointint = 0;
			}

			writeString.Format(L"%d,%d,TC3,%d", i, j, duration);

			myFile.WriteString(writeString);
			writeString = L"\n";
			myFile.WriteString(writeString);
		}
	}

	myFile.Close();

	return;


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

}

