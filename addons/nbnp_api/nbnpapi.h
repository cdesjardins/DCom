#pragma once
#include <string>
#include <windows.h>


class  CNonBlockingNamedPipe
{
public:
    CNonBlockingNamedPipe()
    {
        m_hPipe = INVALID_HANDLE_VALUE;
    }

    ~CNonBlockingNamedPipe()
    {
        if (m_hPipe != INVALID_HANDLE_VALUE)
        {
            CloseHandle(m_hPipe);
            m_hPipe = INVALID_HANDLE_VALUE;
        }
    }

    /*
    ** Open a connection to a named pipe, 
    ** with DCom the pipe name is the last
    ** argument (argv) to the program, it 
    ** should be called as follows:
    ** nbnpapi.Open(argv[argc-1]);
    */
    bool Open(char *szPipeName)
    {
        char szFullPipeName[62];
        BOOL   fSuccess = FALSE; 
        DWORD  dwMode; 
        bool bRet = false;

        sprintf(szFullPipeName, "\\\\.\\pipe\\%s", szPipeName);
        m_hPipe = CreateFile(szFullPipeName, GENERIC_READ | GENERIC_WRITE, 0, NULL,
            OPEN_EXISTING, 0, NULL);

        if (m_hPipe != INVALID_HANDLE_VALUE) 
        {
            dwMode = PIPE_READMODE_BYTE | PIPE_NOWAIT; 
            fSuccess = SetNamedPipeHandleState(m_hPipe, &dwMode, NULL, NULL);
            if (fSuccess == TRUE)
            {
                bRet = true;
            }
        }
        return bRet;
    }

    /*
    ** Read from the named pipe into the
    ** supplied buffer, this is a non blocking
    ** read and it may return with out reading
    ** any data. The return value is the
    ** number of bytes read.
    */
    int Read(char * pData, int nMaxBytes)
    {
        BOOL   fSuccess = FALSE; 
        DWORD  cbRead;

        fSuccess = ReadFile(m_hPipe, pData, nMaxBytes, &cbRead, NULL);
        return cbRead;
    }

    /*
    ** Write to the named pipe from the
    ** supplied buffer, this will attempt
    ** to write the whole buffer within
    ** the amount of time specified. The
    ** return value is the number of bytes
    ** actually written which may be less
    ** than the total number of bytes if
    ** a timeout occurs.
    */
    int Write(char * pData, int nBytes, int nTimeout_ms)
    {
        BOOL   fSuccess = FALSE; 
        DWORD  cbWritten;
        int  cbTotal = 0;
        int nCount = 0;

        do
        {
            fSuccess = WriteFile(m_hPipe, pData, nBytes, &cbWritten, NULL);
            if (fSuccess == TRUE)
            {
                cbTotal += cbWritten;
                pData += cbWritten;
                nBytes -= cbWritten;
            }
            if (nCount++ >= nTimeout_ms)
            {
                break;
            }
            if (cbTotal != nBytes)
            {
                Sleep(1);
            }
        } while ((fSuccess == TRUE) && (cbTotal < nBytes));
        FlushFileBuffers(m_hPipe);
        return cbTotal;
    }

    /*
    ** Wait for the specified string to 
    ** show up on the named pipe
    */
    bool WaitForString(char *szWaitString, int nTimeout_ms, std::string *sOutput)
    {
        bool bRet = true;
        int nRet;
        char szBuf[32];
        int nCount = 0;

        sOutput->clear();
        do
        {
            nRet = Read(szBuf, sizeof(szBuf) - 1);
            if (nRet > 0)
            {
                szBuf[nRet] = 0;
                *sOutput += szBuf;
            }
            if (nCount++ >= nTimeout_ms)
            {
                bRet = false;
                break;
            }
            else
            {
                Sleep(1);
            }
        } while (sOutput->find(szWaitString) == std::string::npos);
        return bRet;
    }

    /*
    ** Easy way to put formated text on the
    ** named pipe
    */
    bool printf(int nTimeout_ms, char *szFormat, ...)
    {
        int nLen;
        char *szData;
        bool bRet = false;

        va_list vl;
        va_start(vl, szFormat);
        nLen = _vscprintf(szFormat, vl);
        szData = (char*)malloc(nLen + 1);
        if (szData)
        {
            vsprintf(szData, szFormat, vl);
            if (Write(szData, nLen, nTimeout_ms) == nLen)
            {
                bRet = true;
            }
            free(szData);
        }
        va_end(vl);
        return bRet;
    }

protected:
    HANDLE m_hPipe;
};
