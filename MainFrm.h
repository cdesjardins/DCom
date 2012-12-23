// MainFrm.h : interface of the CMainFrame class
//


#pragma once
#include "NewConnDlg.h"
#include "RunCmdDlg.h"
#include "FileClipboardDlg.h"
#include "ConfigDlg.h"
#include "DebugDlg.h"

class CMainFrame : public CMDIFrameWnd
{
public:
	CMainFrame();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	virtual ~CMainFrame();
    afx_msg void OnFileMyopen();
    afx_msg void OnFileRuncommand();
    afx_msg void OnClearScreen();
    afx_msg void OnOpenFileClipboard();
    afx_msg void OnUpdateFileRuncommand(CCmdUI *pCmdUI);
    afx_msg void OnUpdateOpenFileClipboard(CCmdUI *pCmdUI);
    afx_msg void OnFileCapture();
    afx_msg void OnUpdateFileCapture(CCmdUI *pCmdUI);
    afx_msg void OnViewOptions();
    afx_msg LRESULT OnFindDialogMessage(WPARAM wParam, LPARAM lParam);
    afx_msg void OnDestroy();
    afx_msg void OnEditFind();
    afx_msg void OnEditRepeat();
    afx_msg void OnUpdateEditFind(CCmdUI *pCmdUI);
    afx_msg void OnUpdateEditRepeat(CCmdUI *pCmdUI);
    afx_msg void OnEditRepeatUp();
    afx_msg void OnUpdateEditRepeatUp(CCmdUI *pCmdUI);
    afx_msg void OnFileSaveTerminals();
    afx_msg void OnFileOpenTerminals();
    afx_msg void OnHelpHelp();
    afx_msg void OnViewDebug();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    SIZE GetLineDimentions(char *pLine, int nLen, CDC *pDC = NULL);
    void InvalidateAll();
    SDComCfg *GetConfig();
    void SetConfig(SDComCfg *sDComCfg);
    char *GetKeyString(int nVirtKey, bool bCtrl, bool bShift) { return m_cfgDlg.GetKeyString(nVirtKey, bCtrl, bShift); };
    void OpenTerminals(CString szFile);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

    TEXTMETRIC m_stFontMetrics;

protected:  // control bar embedded members
	CToolBar    m_wndToolBar;
    CNewConnDlg m_newConnDlg;
    CRunCmdDlg  m_runCmdDlg;
    CFileClipboardDlg m_fileClipboardDlg;
    CConfigDlg m_cfgDlg;
    CDebugDlg m_debugDlg;
    CFindReplaceDialog *m_cFindReplaceDlg;
    CString m_szFindString;
    BOOL m_bFindMatchCase;
    BOOL m_bFindSearchDown;
    static const UINT m_FindDialogMessage;
    UINT m_nTimerID;

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    void FindTextResetAll();
    void DoFindText(BOOL bFindSearchDown);
    BOOL GetTerminalFile(BOOL bOpen, CString *pszFile);

	DECLARE_MESSAGE_MAP()
	DECLARE_DYNAMIC(CMainFrame)
};


