//////////////////////////////////////////////////////////////////////
//
// Redirector - to redirect the input / output of a console
//
// Developer: Jeff Lee
// Dec 10, 2001
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REDIR_H__4FB57DC3_29A3_11D5_BB60_006097553C52__INCLUDED_)
#define AFX_REDIR_H__4FB57DC3_29A3_11D5_BB60_006097553C52__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CRedirector : public CObject  
{
public:
    CRedirector();
    virtual ~CRedirector();

protected:
    LONG m_atomic_data;
    bool m_bStdin;
    bool m_bPipein;

    HANDLE m_hPipe;
    HANDLE m_hDebugPipe;

    HANDLE m_hThread;		// thread to receive the output of the child process
    HANDLE m_hEvtStop;		// event to notify the redir thread to exit
    DWORD m_dwThreadId;		// id of the redir thread

    HANDLE m_hStdinWrite;   // write end of child's stdin pipe
    HANDLE m_hStdoutRead;   // read end of child's stdout pipe

    HANDLE m_hChildProcess;
    BOOL LaunchChild(LPCTSTR pszCmdLine, LPCTSTR lpszDirectory,
        HANDLE hStdOut, HANDLE hStdIn, HANDLE hStdErr);
    BOOL RedirectOutput(HANDLE hRedirector, BOOL bStdOut);
    void DestroyHandle(HANDLE& rhObject);
    static DWORD WINAPI OutputThread(LPVOID lpvThreadParam);
    virtual void WriteStdOut(LPCSTR pszOutput, DWORD dwChars) = 0;
    void OpenError(HANDLE hStdoutReadTmp, HANDLE hStdoutWrite, 
        HANDLE hStderrWrite, HANDLE hStdinWriteTmp, HANDLE hStdinRead);
    BOOL BuildPipe(CString szPipeName, HANDLE *hPipe);
    BOOL WriteOutput(HANDLE hOut, LPCTSTR pszString, DWORD nLen, bool bWriteIt);
    void PostError();
public:
    virtual BOOL Open(LPCTSTR pszCmdLine, LPCTSTR lpszDirectory, bool bStdin, bool bPipein);
    virtual void Close();
    virtual BOOL PrintS(LPCTSTR pszString, int nLen);
};

#endif // !defined(AFX_REDIR_H__4FB57DC3_29A3_11D5_BB60_006097553C52__INCLUDED_)
