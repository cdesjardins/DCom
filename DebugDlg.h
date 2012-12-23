#pragma once
#include "afxwin.h"
#include "SizeDialog.h"


// CDebugDlg dialog

class CDebugDlg : public CSizeDialog
{
	DECLARE_DYNAMIC(CDebugDlg)
    DECLARE_EASYSIZE

public:
	CDebugDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDebugDlg();
    void DebugOutput(const char *szFormat, ...);
    afx_msg void OnBnClickedDebugPause();
    virtual BOOL OnInitDialog();
    virtual char *GetSectionName()
    {
        return "Debug Dialog";
    };
//    virtual int GetHelpId() { return IDH_DBG_NAMED_PIPE; };

// Dialog Data
	enum { IDD = IDD_DEBUG_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
    CRichEditCtrl m_debugEdit;
    bool m_bPaused;
    long m_nChars;
    CButton m_pauseButton;
    CFont m_font;
public:
    afx_msg void OnBnClickedDebugClear();
};
