// DComDoc.h : interface of the CDComDoc class
//


#pragma once
#include "DComView.h"
#include "Redir.h"
#include "TgtIntf.h"
#include "NewConnDlg.h"
#include "scrollbuff.h"

class CMyRedirector : public CRedirector
{
public:
    CMyRedirector();
    BOOL Open(LPCTSTR pszCmdLine, LPCTSTR lpszDirectory, CDComDoc* pDoc, bool bStdin, bool bPipein);
    void Close();
    BOOL IsRunning();
protected:
    void WriteStdOut(LPCSTR pszOutput, DWORD dwChars);
private:
    CDComDoc *m_pDoc;
};

class CMyMultiDocTemplate : public CMultiDocTemplate
{
public:
    CDocument* OpenDocumentFile(SDialingDir *psDialingDir);
};

class CDComDoc : public CDocument
{
protected: // create from serialization only
    void InjectAnsi(CDComDoc *pDoc);

	CDComDoc();
	DECLARE_MESSAGE_MAP()
	DECLARE_DYNCREATE(CDComDoc)

    CWinThread *m_cTgtIntfThread;
    char m_sTgtIOBuffer[512];
    char m_szOpenFileName[128];
    CMyRedirector m_cCmdRedirector;
    CScrollBufL1 m_cLineBuffer;
    bool m_bFileCapture;
    int m_nContextMenuId;
    HANDLE m_hThreadTerm;
    HANDLE m_hThreadTerminated;
    SSelection m_sFindStartingLine;
    SDialingDir m_sDialingDir;
    CString m_szExtendedTitle;

    static UINT TgtIntfThread(LPVOID arg);
    virtual BOOL SaveModified();
    virtual CDComView *GetView();
    int m_nEmulation;

public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
    virtual BOOL RunCommand(LPCTSTR lpszCommand, LPCTSTR lpszDirectory, bool bStdin, bool bPipein, bool bUserin);
    virtual void StopRunning() {m_cCmdRedirector.Close();};
    virtual BOOL IsRunning() {return m_cCmdRedirector.IsRunning();};
    virtual ~CDComDoc();
    virtual void SetTitle();
    virtual void ExtendWindowTitle();
    virtual void ClearScreen();
    virtual BOOL OnOpenDocument(SDialingDir *psDialingDir);
    virtual void OnCloseDocument();
    virtual void OnFileCapture(CWnd *pParent);
    virtual void FindText(CString szFindString, BOOL bFindMatchCase, BOOL bFindSearchDown);
    virtual void FindTextReset();
    virtual void SetConfig(SDComCfg *sDComCfg);
    virtual SDialingDir *GetDialingDir() { return &m_sDialingDir; };
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

    TgtIntf *m_pTgtIntf;
};


