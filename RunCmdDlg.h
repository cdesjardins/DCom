#pragma once
#include "afxwin.h"
#include "SizeDialog.h"

// CRunCmdDlg dialog

class CRunCmdDlg : public CSizeDialog
{
	DECLARE_DYNAMIC(CRunCmdDlg)
    DECLARE_EASYSIZE

public:
	CRunCmdDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRunCmdDlg();

// Dialog Data
	enum { IDD = IDD_COMMAND_DIALOG };
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedOk();
    afx_msg void OnCbnSelchangeCommandCombo();
    afx_msg void OnBnClickedRunBrowse();
    afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
    afx_msg void OnBnClickedRunBrowseWd();
    virtual char *CRunCmdDlg::GetSectionName();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    void AddCommandToRegistry(CString szCommand, CString szDirectory, bool bStdin, bool bPipein, bool bUser);
    void GetCommandsFromRegistry();
    int IsInRegistry(char *szKey, char *szCommand, char *szNum);
    virtual int GetHelpId() {return IDH_RUN_COMMAND;};

	DECLARE_MESSAGE_MAP()
    CButton m_stdin;
    CButton m_pipein;
    CButton m_userin;
    CComboBox m_commandCombo;
    CComboBox m_workingDirCombo;
};
