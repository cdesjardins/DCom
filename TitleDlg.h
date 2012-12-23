#pragma once
#include "afxwin.h"


// CTitleDlg dialog

class CTitleDlg : public CDialog
{
	DECLARE_DYNAMIC(CTitleDlg)

public:
	CTitleDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTitleDlg();

// Dialog Data
	enum { IDD = IDD_TITLE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    CEdit m_cTitleEdit;
    afx_msg void OnBnClickedOk();
    CString m_szTitleText;
    virtual BOOL OnInitDialog();
};
