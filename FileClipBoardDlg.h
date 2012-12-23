#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "SizeDialog.h"
#include <list>

class CMyListCtrl : public CListCtrl
{
public:
    CMyListCtrl();
    void SendSelectedText();

protected:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnLvnEndlabeledit(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult);
    void SendText(int nItem);

    std::list<int> m_vecSelection;
};

// CFileClipboardDlg dialog

class CFileClipboardDlg : public CSizeDialog
{
	DECLARE_DYNAMIC(CFileClipboardDlg)
    DECLARE_EASYSIZE
public:

	CFileClipboardDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFileClipboardDlg();

// Dialog Data
	enum { IDD = IDD_FILE_CLIPBOARD_DIALOG };
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedFileClipboardOpen();
    afx_msg void OnBnClickedFileClipboardSave();
    afx_msg void OnBnClickedFileClipboardSend();
    afx_msg void OnBnClickedFileClipboardRemove();
    afx_msg void OnDestroy();
    afx_msg void OnLvnKeydownFileClipboardList(NMHDR *pNMHDR, LRESULT *pResult);
    virtual char *GetSectionName(); 

    CButton m_btNewLine;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    BOOL GetFileClipboardFilename(BOOL bOpen, CString *szFilename);
    void ReadFileClipboard(CString szFilename);
    virtual int GetHelpId() {return IDH_FILE_CLIPBOARD;};
    virtual void OnOK();

	DECLARE_MESSAGE_MAP()
    CMyListCtrl m_cFileClipboardList;
};
