
// LongIntegersDlg.h : header file
//

#pragma once
#include "afxwin.h"

#include "LongInteger.h"
#include "afxcmn.h"

// CLongIntegersDlg dialog
class CLongIntegersDlg : public CDialogEx
{
// Construction
public:
	CLongIntegersDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_LONGINTEGERS_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg BOOL OnSetCursor(CWnd*, UINT, UINT);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
//	CEdit mEdit1;
//	CStatic m_BigIntOutput;
//	CEdit m_ChangeNumber;
	afx_msg void OnClickedIdadd();
//	afx_msg void OnUpdateEdit1();

private:
	LongInteger m_longInt;
	CWinThread* m_procthread;
public:
	afx_msg void OnClickedIdminus();
	afx_msg void OnClickedIdmultiply();
	afx_msg void OnClickedIdtoclip();
	CRichEditCtrl m_RichEdit1;
	afx_msg void OnUpdateRichedit1();
	CRichEditCtrl m_RichEdit2;
	CRichEditCtrl m_OutputNumber;
	afx_msg void OnChangeRichedit1();
	afx_msg void OnClickedIdpower();
	afx_msg void OnClickedIdarrow();
	afx_msg void OnClickedIddivide();
	CRichEditCtrl m_ArrowLevel;

	static UINT StartPowerWork(LPVOID);
	static UINT StartMultWork(LPVOID);
	static UINT StartDivWork(LPVOID);
};
