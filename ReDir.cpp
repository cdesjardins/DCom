//////////////////////////////////////////////////////////////////////
//
// Redirector - to redirect the input / output of a console
//
// Developer: Jeff Lee
// Dec 10, 2001
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Redir.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/*#define _TEST_REDIR*/

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRedirector::CRedirector() :
    m_hStdinWrite(INVALID_HANDLE_VALUE),
    m_hStdoutRead(INVALID_HANDLE_VALUE),
    m_hChildProcess(INVALID_HANDLE_VALUE),
    m_hThread(INVALID_HANDLE_VALUE),
    m_hEvtStop(INVALID_HANDLE_VALUE),
    m_hPipe(INVALID_HANDLE_VALUE),
    m_hDebugPipe(INVALID_HANDLE_VALUE),
    m_dwThreadId(0),
    m_bStdin (false),
    m_bPipein (false),
    m_atomic_data(0)
{
}

CRedirector::~CRedirector()
{
    Close();
}

//////////////////////////////////////////////////////////////////////
// CRedirector implementation
//////////////////////////////////////////////////////////////////////

void CRedirector::PostError()
{
    DWORD dwOsErr = ::GetLastError();
    LPTSTR szMsg;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, dwOsErr, 0,(LPTSTR)&szMsg, 0, NULL);

    AfxMessageBox(szMsg);
}

void CRedirector::OpenError(HANDLE hStdoutReadTmp, HANDLE hStdoutWrite, 
    HANDLE hStderrWrite, HANDLE hStdinWriteTmp, HANDLE hStdinRead)
{
    PostError();
    DestroyHandle(hStdoutReadTmp);
    DestroyHandle(hStdoutWrite);
    DestroyHandle(hStderrWrite);
    DestroyHandle(hStdinWriteTmp);
    DestroyHandle(hStdinRead);
    Close();
}

BOOL CRedirector::BuildPipe(CString szPipeName, HANDLE *hPipe)
{
    BOOL   bRet = TRUE; 
    CString szFullPipeName;

    szFullPipeName.Format("\\\\.\\pipe\\%s", szPipeName);

    (*hPipe) = CreateNamedPipe(szFullPipeName.GetBuffer(), 
        PIPE_ACCESS_DUPLEX, 
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
        PIPE_UNLIMITED_INSTANCES, 512, 512, 0, NULL);
    if ((*hPipe) == INVALID_HANDLE_VALUE)
    {
        bRet = FALSE;
    }
    return bRet;
}

BOOL CRedirector::Open(LPCTSTR pszCmdLine, LPCTSTR lpszDirectory, bool bStdin, bool bPipein)
{
    CString szCmdLine;
    CString szPipeName;
    CString szDebugPipeName;
    HANDLE hStdoutReadTmp = NULL;
    HANDLE hStdoutWrite = NULL;
    HANDLE hStderrWrite = NULL;
    HANDLE hStdinWriteTmp = NULL;
    HANDLE hStdinRead = NULL;
    SECURITY_ATTRIBUTES sa;

    m_bStdin = bStdin;
    m_bPipein = bPipein;

    hStdoutReadTmp = NULL;
    hStdoutWrite = hStderrWrite = NULL;
    hStdinWriteTmp = NULL;
    hStdinRead = NULL;

    // Set up the security attributes struct.
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    BOOL bOK = FALSE;
    // Create a child stdout pipe.
    if (!::CreatePipe(&hStdoutReadTmp, &hStdoutWrite, &sa, 0))
    {
        OpenError(hStdoutReadTmp, hStdoutWrite, hStderrWrite, hStdinWriteTmp, hStdinRead);
    }
    else if (!::CreatePipe(&hStdinRead, &hStdinWriteTmp, &sa, 0))
    {
        OpenError(hStdoutReadTmp, hStdoutWrite, hStderrWrite, hStdinWriteTmp, hStdinRead);
    }
    else if (!::DuplicateHandle(::GetCurrentProcess(), hStdoutWrite,
        ::GetCurrentProcess(), &hStderrWrite, 0, TRUE, DUPLICATE_SAME_ACCESS))
    {
        OpenError(hStdoutReadTmp, hStdoutWrite, hStderrWrite, hStdinWriteTmp, hStdinRead);
    }
    else if (!::DuplicateHandle(::GetCurrentProcess(), hStdoutReadTmp,
        ::GetCurrentProcess(), &m_hStdoutRead,0, FALSE, DUPLICATE_SAME_ACCESS))
    {
        OpenError(hStdoutReadTmp, hStdoutWrite, hStderrWrite, hStdinWriteTmp, hStdinRead);
    }
    else if (!::DuplicateHandle(::GetCurrentProcess(), hStdinWriteTmp,
        ::GetCurrentProcess(), &m_hStdinWrite, FILE_GENERIC_WRITE, FALSE, 0))
    {
        OpenError(hStdoutReadTmp, hStdoutWrite, hStderrWrite, hStdinWriteTmp, hStdinRead);
    }
    else
    {

        // Close inheritable copies of the handles we do not want to
        // be inherited.
        DestroyHandle(hStdoutReadTmp);
        DestroyHandle(hStdinWriteTmp);

        szPipeName.Format("%x", GetCurrentThreadId());
        szDebugPipeName.Format("%s_debug", szPipeName);
        if ((BuildPipe(szPipeName, &m_hPipe) == FALSE) || (BuildPipe(szDebugPipeName, &m_hDebugPipe) == FALSE))
        {
            OpenError(hStdoutReadTmp, hStdoutWrite, hStderrWrite, hStdinWriteTmp, hStdinRead);
        }
        else
        {
            szCmdLine.Format("%s %s", pszCmdLine, szPipeName);
            // launch the child process
            if (!LaunchChild(szCmdLine.GetBuffer(), lpszDirectory,
                hStdoutWrite, hStdinRead, hStderrWrite))
            {
                OpenError(hStdoutReadTmp, hStdoutWrite, hStderrWrite, hStdinWriteTmp, hStdinRead);
            }
            else
            {
                // Child is launched. Close the parents copy of those pipe
                // handles that only the child should have open.
                // Make sure that no handles to the write end of the stdout pipe
                // are maintained in this process or else the pipe will not
                // close when the child process exits and ReadFile will hang.
                DestroyHandle(hStdoutWrite);
                DestroyHandle(hStdinRead);
                DestroyHandle(hStderrWrite);

                // Launch a thread to receive output from the child process.
                m_hEvtStop = ::CreateEvent(NULL, TRUE, FALSE, NULL);
                m_hThread = ::CreateThread(NULL, 0, OutputThread, this, 0, &m_dwThreadId);
                if (m_hThread <= 0)
                {
                    OpenError(hStdoutReadTmp, hStdoutWrite, hStderrWrite, hStdinWriteTmp, hStdinRead);
                }
                else
                {
                    bOK = TRUE;
                }
            }
        }
    }
    return bOK;
}

void CRedirector::Close()
{
    /*
    ** This function is called from the destructor and if the thread 
    ** ends on its own, there is also a call to close if you choose to 
    ** end the thread early. Sometimes it is called from multiple places 
    ** in which case we only want it to actually close once. In order to prevent
    ** multiple calls from happening at the same time, I do an atomic operation
    ** here which will prevent one of the calls from clobbering the other...
    */
    if (InterlockedIncrement(&m_atomic_data) == 1)
    {
        if (m_dwThreadId)
        {
            m_bStdin = false;
            m_bPipein = false;

            if (m_hThread > 0)
            {
                // this function might be called from redir thread
                if (::GetCurrentThreadId() != m_dwThreadId)
                {
                    ::SetEvent(m_hEvtStop);
                    if (::WaitForSingleObject(m_hThread, 1000) == WAIT_TIMEOUT)
                    {
                        ::TerminateThread(m_hThread, -2);
                    }
                }

                DestroyHandle(m_hThread);
            }
            if (m_hChildProcess > 0)
            {
                TerminateProcess(m_hChildProcess, 0xdeadd00d);
            }
            DestroyHandle(m_hEvtStop);
            DestroyHandle(m_hChildProcess);
            DestroyHandle(m_hStdinWrite);
            DestroyHandle(m_hStdoutRead);
            DestroyHandle(m_hPipe);
            DestroyHandle(m_hDebugPipe);
            m_dwThreadId = 0;
        }
        m_atomic_data = 0;
    }
}

BOOL CRedirector::WriteOutput(HANDLE hOut, LPCTSTR pszString, DWORD nLen, bool bWriteIt)
{
    DWORD dwTotal = 0;
    BOOL bRet = FALSE;
    DWORD dwWritten;

    if (bWriteIt)
    {
        if (hOut > 0)
        {
            do
            {
                bRet = ::WriteFile(hOut, (LPCTSTR)pszString, nLen, &dwWritten, NULL);
                if (bRet)
                {
                    dwTotal += dwWritten;
                }
            } while ((dwTotal < nLen) && (bRet == TRUE));
        }
    }
    else
    {
        bRet = TRUE;
    }
    return bRet;
}

BOOL CRedirector::PrintS(LPCTSTR pszString, int nLen)
{
    BOOL bRetPipe = FALSE, bRetStd = FALSE;
    bRetPipe = WriteOutput(m_hPipe, pszString, nLen, m_bPipein);
    bRetStd = WriteOutput(m_hStdinWrite, pszString, nLen, m_bStdin);
    return (bRetPipe == TRUE) && (bRetStd == TRUE);
}

BOOL CRedirector::LaunchChild(LPCTSTR pszCmdLine, LPCTSTR lpszDirectory, HANDLE hStdOut, HANDLE hStdIn, HANDLE hStdErr)
{
    PROCESS_INFORMATION pi;
    STARTUPINFO si;

    ASSERT(::AfxIsValidString(pszCmdLine));

    // Set up the start up info struct.
    ::ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.hStdOutput = hStdOut;
    si.hStdInput = hStdIn;
    si.hStdError = hStdErr;
    si.wShowWindow = SW_HIDE;
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

    // Note that dwFlags must include STARTF_USESHOWWINDOW if we
    // use the wShowWindow flags. This also assumes that the
    // CreateProcess() call will use CREATE_NEW_CONSOLE.

    // Launch the child process.
    if ((lpszDirectory) && (strlen(lpszDirectory) == 0))
    {
        lpszDirectory = NULL;
    }

    if (!::CreateProcess(NULL, (LPTSTR)pszCmdLine, NULL, NULL, TRUE,
        CREATE_NEW_CONSOLE, NULL, lpszDirectory, &si, &pi))
    {
        return FALSE;
    }
    m_hChildProcess = pi.hProcess;
    // Close any unuseful handles
    ::CloseHandle(pi.hThread);
    return TRUE;
}

BOOL CRedirector::RedirectOutput(HANDLE hRedirector, BOOL bStdOut)
{
    char szOutput[256];
    DWORD dwRead = 0;
    DWORD dwAvail = 0;
    BOOL bRet = TRUE;
    void DebugOutput(const char *szFormat, ...);

    if (!::PeekNamedPipe(hRedirector, NULL, 0, NULL, &dwAvail, NULL))
    {
        bRet = FALSE;
    }
    else
    {
        if (dwAvail > 0)
        {
            if (!::ReadFile(hRedirector, szOutput, sizeof(szOutput) - 1, &dwRead, NULL))
            {
                DWORD dwError = ::GetLastError();
                if (dwError == ERROR_BROKEN_PIPE || // pipe has been ended
                    dwError == ERROR_NO_DATA)       // pipe closing in progress
                {
                    bRet = FALSE;
                }
            }
            if (dwRead > 0)
            {
                szOutput[dwRead] = 0;
                if (bStdOut)
                {
                    WriteStdOut(szOutput, dwRead);
                }
                else
                {
                    DebugOutput(szOutput);
                }
            }
        }
    }
    return bRet;
}

void CRedirector::DestroyHandle(HANDLE& rhObject)
{
    if (rhObject > NULL)
    {
        ::CloseHandle(rhObject);
        rhObject = INVALID_HANDLE_VALUE;
    }
}

// thread to receive output of the child process
DWORD WINAPI CRedirector::OutputThread(LPVOID lpvThreadParam)
{
    HANDLE aHandles[2];
    CRedirector* pRedir = (CRedirector*) lpvThreadParam;

    aHandles[0] = pRedir->m_hChildProcess;
    aHandles[1] = pRedir->m_hEvtStop;

    for (;;)
    {
        pRedir->RedirectOutput(pRedir->m_hStdoutRead, TRUE);
        pRedir->RedirectOutput(pRedir->m_hPipe, TRUE);
        pRedir->RedirectOutput(pRedir->m_hDebugPipe, FALSE);
        // check if the child process has terminated.
        DWORD dwRc = ::WaitForMultipleObjects((sizeof(aHandles) / sizeof(aHandles[0])), aHandles, FALSE, 1);
        if ((WAIT_OBJECT_0 == dwRc) ||   // the child process ended
            (WAIT_OBJECT_0 + 1 == dwRc)) // m_hEvtStop was signalled
        {
            break;
        }
    }

    // close handles
    pRedir->Close();
    return 1;
}
