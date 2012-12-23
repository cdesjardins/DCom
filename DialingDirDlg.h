#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "SizeDialog.h"

#define DCOM_DIALINGDIR_SECTION   "DialingDir"
#define DCOM_DIALINGDIR_PATH      "Path"

// CDialingDirDlg dialog
#define WM_DIALING_DIR_OK    (WM_USER + 20)

class CDialingDirDlg : public CSizeDialog
{
	DECLARE_DYNAMIC(CDialingDirDlg)
    DECLARE_EASYSIZE
public:
	CDialingDirDlg(CWnd* pParent = NULL);   // standard constructor
    virtual BOOL OnInitDialog();
	virtual ~CDialingDirDlg();
    SDialingDir GetSelected();
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedOpenDialingDir();
    virtual char *GetSectionName();
	enum { IDD = IDD_DIALING_DIR_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    void ParseDialingDir();
    void RecurseDialingDir(CStdioFile *fDialingDir, int nLevel, HTREEITEM hParent);
    int GetCurrentLevel();
    void SetupIcons(HTREEITEM hTreeItem);

	DECLARE_MESSAGE_MAP()
    CTreeCtrl m_dialingDirTree;
    std::vector<SDialingDir> m_sDialingDir;
    SDialingDir m_sSelectedDialingDir;
    char m_szInput[256];
    CString m_szDialingDirFilename;
    CButton m_cancelButton;
    CButton m_okButton;
    CButton m_openDialingdirButton;
    CImageList m_imageList;
    int m_telnetBmIndex;
    int m_sshBmIndex;
    int m_dotBmIndex;


public:
    afx_msg void OnNMDblclkDialingDirTree(NMHDR *pNMHDR, LRESULT *pResult);
};
