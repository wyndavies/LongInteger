
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


	// Test a different approach to division and see if it is faster
	LongInteger liVal, liDiv;
	LongInteger liQ, liM, liQ2;
	LongInteger liNQ, liNM;


	liVal = CString(L"12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890");
	liDiv = CString(L"888");

	liNQ = liVal / liDiv;
	liNM = liVal % liDiv;


	// We will run a test. If the divisor is more than 1 digit in size less than the value, multiply the 
	// divisor by 256 until it is (keeping a count of how many times
	// Then loop that many times:
	//	shift the quotient left 1
	//  shift the (inflated) divisor right 1
	//  divide the modulus by the divisor
	//  add the quotient of this to the original quotient

	UINT count = 0;
	while (liDiv.getSize() < (liVal.getSize() - 1)) {
		liDiv.bitshiftleft(8);
		count++;
	}
	liQ = liVal / liDiv;
	liM = liVal % liDiv;
	while (count > 0) {
		liQ.bitshiftleft(8);
		liDiv.bitshiftright(8);
		liQ2 = liM / liDiv;
		liM = liM % liDiv;
		liQ += liQ2;
		count--;
	}

	CString strQ = liQ.toDecimal();
	CString strM = liM.toDecimal();
	CString strNQ = liNQ.toDecimal();
	CString strNM = liNM.toDecimal();
	bool workedQ = liQ == liNQ;
	bool workedM = liM == liNM;


	// Now try some random values to see how it works.
	UINT base = 1000;
	UINT increment = 500;
	for (UINT i = 6000; i <= 10000; i += base) {
		byte* array1 = new byte[i];
		memset(array1, 111, i);
		liVal.assignByteArray(array1, i);
		delete array1;
		for (UINT j = 10; j <= 100; j += base) {
			byte* array2 = new byte[j];
			memset(array2, 55, j);
			liDiv.assignByteArray(array2, j);
			delete array2;
			liDiv = 10;

//			liNQ = liVal / liDiv;
//			liNM = liVal % liDiv;

			for (UINT k = 100; k < 1000; k++) {
				increment = k;

				UINT shift = 0;
				if (liVal.getSize() > (liDiv.getSize() * 2) && liDiv.getSize() > 0) {
					shift = liVal.getSize() - liDiv.getSize() - 2;
					liDiv.bitshiftleft(8 * shift);
				}
				liQ = liVal / liDiv;
				liM = liVal % liDiv;
				/*
				while (shift > 0 && shift > j) {
					UINT subshift = shift;
					// If the shifted liDiv is going to be less than half the size of liM, change the shift amount
					// to make it slightly more than half the size of liDiv
					// So liDiv/2 = liM - shift => =shift = lim - (liDiv/2)
					if ((liM.getSize() - 10) > ((liDiv.getSize() - subshift) / 2))
						subshift = (liM.getSize() - (liDiv.getSize() / 2)) + 10;
					liQ.bitshiftleft(8 * subshift);
					liDiv.bitshiftright(8 * subshift);
					liQ2 = liM / liDiv;
					liM = liM % liDiv;
					liQ += liQ2;
					shift -= subshift;
				}
				if (shift > 0)
				{
					liQ.bitshiftleft(8 * shift);
					liDiv.bitshiftright(8 * shift);
					liQ2 = liM / liDiv;
					liM = liM % liDiv;
					liQ += liQ2;
				}*/
				while (shift > j && shift > increment) {
					UINT subshift = increment;
					liQ.bitshiftleft(8 * subshift);
					liDiv.bitshiftright(8 * subshift);
					liQ2 = liM / liDiv;
					liM = liM % liDiv;
					liQ += liQ2;
					shift -= subshift;
				}
				if (shift > 0)
				{
					liQ.bitshiftleft(8 * shift);
					liDiv.bitshiftright(8 * shift);
					if (liDiv.getSize() > 0) {
						liQ2 = liM / liDiv;
						liM = liM % liDiv;
						liQ += liQ2;
					}
				}




				strQ = liQ.toDecimal();
				strM = liM.toDecimal();
				strNQ = liNQ.toDecimal();
				strNM = liNM.toDecimal();
				workedQ = liQ == liNQ;
				workedM = liM == liNM;
			}

		}
	}








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
