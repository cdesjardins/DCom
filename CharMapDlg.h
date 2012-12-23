#pragma once

#include "SizeDialog.h"
#include "afxcmn.h"

#define WM_CHARMAP_OK    (WM_USER + 30)

class CCharMapDlg : public CSizeDialog
{
	DECLARE_DYNAMIC(CCharMapDlg)
    DECLARE_EASYSIZE

public:
	CCharMapDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCharMapDlg();
    virtual char *GetSectionName(); 
    virtual BOOL OnInitDialog();
	enum { IDD = IDD_CHARMAP_DIALOG };
    char GetSelectedChar();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
    CListCtrl m_cCharMapList;

public:
    afx_msg void OnBnClickedOk();
};
