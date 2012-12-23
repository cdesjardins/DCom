#include "StdAfx.h"
#include "TgtIntf.h"
#include "scrollbuff.h"
#include <process.h>
/* 
** If telnet or ssh to a remote server and it is unix based and you see
** ^H every time you issue a backspace, then add stty erase ^H to your
** startup script (.cshrc) or the like.
*/

#define TGT_BACKSPACE       0x08

TgtIntf::TgtIntf(void)
{
    m_nTotalTx = 0;
    m_nTotalRx = 0;
}

TgtIntf::~TgtIntf(void)
{
}

/******************************************************************************
**
**  SSH
**
******************************************************************************/
#include "libssh/callbacks.h"

TgtSshIntf::TgtSshIntf()
{
    m_sshSession = 0;
    m_sshChannel = 0;
    m_cbUserPass.SetMaxNumLines(2);
    ResetAuthData(true, TGT_SSH_AUTH_STATE_USER_PROMPT);
}

TgtSshIntf::~TgtSshIntf()
{

}

void TgtSshIntf::ResetAuthData(bool bClearUserPass, eTgtSshAuthState eAuthState)
{
    m_cbUserPass.ClearAllLines();
    if (bClearUserPass)
    {
        memset(m_szUsername, 0, sizeof(m_szUsername));
        memset(m_szPassword, 0, sizeof(m_szPassword));
    }
    memset(m_szAuthInput, 0, sizeof(m_szAuthInput));
    m_nAuthShown = 0;
    m_nAuthLen = 0;
    m_eAuthenticated = eAuthState;
}

char * TgtSshIntf::TgtMakeConnection()
{
    return NULL;
}

int TgtSshIntf::TgtDisconnect()
{
    if (m_sshChannel)
    {
        channel_free(m_sshChannel);
        m_sshChannel = 0;
    }
    if (m_sshSession)
    {
        ssh_disconnect(m_sshSession);
    }
    ssh_finalize();
    if (m_sshSession)
    {
        ssh_free(m_sshSession);
        m_sshSession = 0;
    }
    return 0;
}
/*
void DebugOutput(const char *szFormat, ...);

void TgtLog(ssh_session session, int priority, const char *message, void *userdata)
{
    DebugOutput("%s\r\n", message);
}
*/
int TgtSshIntf::TgtAuthenticate(char *szReadData, int nMaxBytes)
{
    int nTmp;

    strcpy(szReadData, "\r\n");
    m_sshSession = ssh_new();
    if (m_sshSession == NULL)
    {
        strcat(szReadData, "Unable to allocate memory for SSH connection");
        ResetAuthData(true, TGT_SSH_AUTH_STATE_USER_PROMPT);
    }
    else
    {
        ssh_options_set(m_sshSession, SSH_OPTIONS_HOST, m_sTgtConnection.m_szServerName);
        ssh_options_set(m_sshSession, SSH_OPTIONS_PORT, (void*)&m_sTgtConnection.m_nPort);
        ssh_options_set(m_sshSession, SSH_OPTIONS_COMPRESSION_C_S, "none");
        ssh_options_set(m_sshSession, SSH_OPTIONS_COMPRESSION_S_C, "none");
        ssh_options_set(m_sshSession, SSH_OPTIONS_CIPHERS_C_S,"aes128-cbc,aes192-cbc,aes256-cbc,3des-cbc,3des-cbc-ssh1,blowfish-cbc");
        ssh_options_set(m_sshSession, SSH_OPTIONS_CIPHERS_S_C,"aes128-cbc,aes192-cbc,aes256-cbc,3des-cbc,3des-cbc-ssh1,blowfish-cbc");
        /*
        {
            ssh_callbacks_struct cb;
            memset(&cb, 0, sizeof(struct ssh_callbacks_struct));
            cb.log_function = TgtLog;

            ssh_callbacks_init(&cb);
            ssh_set_callbacks(m_sshSession, &cb);
        }
        nTmp = SSH_LOG_FUNCTIONS;
        ssh_options_set(m_sshSession, SSH_OPTIONS_LOG_VERBOSITY, &nTmp);
        */
        nTmp = 1;
        ssh_options_set(m_sshSession, SSH_OPTIONS_SSH2, (void*)&nTmp);
        if (ssh_connect(m_sshSession))
        {
            strcat(szReadData, "Unable to connect\r\n");
            ResetAuthData(true, TGT_SSH_AUTH_STATE_USER_PROMPT);
            TgtDisconnect();
        }
        else if (ssh_userauth_password(m_sshSession, m_szUsername, m_szPassword) != SSH_AUTH_SUCCESS)
        {
            strcat(szReadData, "Invalid Username or Password\r\n");
            ResetAuthData(true, TGT_SSH_AUTH_STATE_USER_PROMPT);
            TgtDisconnect();
        }
        else
        {
            m_sshChannel = channel_new(m_sshSession);
            if (m_sshChannel == NULL)
            {
                strcat(szReadData, "Unable to allocate SSH channel\r\n");
                ResetAuthData(true, TGT_SSH_AUTH_STATE_USER_PROMPT);
                TgtDisconnect();
            }
            else if (channel_open_session(m_sshChannel) != SSH_OK)
            {
                strcat(szReadData, "Unable to open SSH session\r\n");
                ResetAuthData(true, TGT_SSH_AUTH_STATE_USER_PROMPT);
                TgtDisconnect();
            }
            else
            {
                channel_request_pty(m_sshChannel);
                channel_request_shell(m_sshChannel);
                m_eAuthenticated = TGT_SSH_AUTH_STATE_DONE;
            }
        }
    }
    return strlen(szReadData);
}

int TgtSshIntf::TgtAuthenticationFeedback(char *szReadData, char *szInput, bool bPassword)
{
    int nRet = 0;
    while (m_nAuthLen > m_nAuthShown)
    {
        if (szInput[m_nAuthShown] == TGT_BACKSPACE)
        {
            m_nAuthLen -= 2;
            if (m_nAuthLen < 0)
            {
                m_nAuthLen = 0;
            }
            else
            {
                szReadData[nRet++] = TGT_BACKSPACE;
                szReadData[nRet++] = ' ';
                szReadData[nRet++] = TGT_BACKSPACE;
                m_nAuthShown--;
            }
        }
        else
        {
            if (bPassword)
            {
                szReadData[nRet++] = '*';
            }
            else
            {
                szReadData[nRet++] = szInput[m_nAuthShown];
            }
            m_nAuthShown++;
        }
    }
    return nRet;
}

int TgtSshIntf::TgtAuthenticatePrompt(char *szReadData, int nMaxBytes)
{
    int nRet = 0;
    switch (m_eAuthenticated)
    {
    case TGT_SSH_AUTH_STATE_USER_PROMPT:
        m_eAuthenticated = TGT_SSH_AUTH_STATE_USER_ACCEPT;
        strncpy(szReadData, "Username: ", nMaxBytes);
        nRet = strlen(szReadData);
        m_nAuthShown = 0;
        break;
    case TGT_SSH_AUTH_STATE_USER_ACCEPT:
        nRet = TgtAuthenticationFeedback(szReadData, m_szAuthInput, false);
        break;
    case TGT_SSH_AUTH_STATE_PASS_PROMPT:
        m_eAuthenticated = TGT_SSH_AUTH_STATE_PASS_ACCEPT;
        strncpy(szReadData, "\r\nPassword: ", nMaxBytes);
        nRet = strlen(szReadData);
        m_nAuthShown = 0;
        break;
    case TGT_SSH_AUTH_STATE_PASS_ACCEPT:
        nRet = TgtAuthenticationFeedback(szReadData, m_szAuthInput, true);
        break;
    case TGT_SSH_AUTH_STATE_PASS_AUTHENTICATE:
        nRet = TgtAuthenticate(szReadData, nMaxBytes);
        break;
    }
    return nRet;
}

int TgtSshIntf::TgtAuthenticateAcceptInput(char *szWriteData, int nBytes, char *szFinalInput, eTgtSshAuthState eNextState)
{
    bool bValid;
    SFullLine *flInput;
    int nWriteIndex = 0;
    bool bNewLine = false;

    for (nWriteIndex = 0; nWriteIndex < nBytes; nWriteIndex++)
    {
        if (szWriteData[nWriteIndex] == '\r')
        {
            bNewLine = true;
            break;
        }
        m_szAuthInput[m_nAuthLen++] = szWriteData[nWriteIndex];
    }

    if (bNewLine)
    {
        m_cbUserPass.AddText(m_szAuthInput, strlen(m_szAuthInput));
        m_cbUserPass.Enter();
        flInput = m_cbUserPass.GetLine(0, &bValid);
        if (bValid)
        {
            strncpy(szFinalInput, flInput->szLine, m_cbUserPass.GetCaretCol());
        }
        m_cbUserPass.Exit();
        ResetAuthData(false, eNextState);
    }
    return nBytes;
}

int TgtSshIntf::TgtAuthenticateAccept(char *szWriteData, int nBytes)
{
    int nRet = 0;

    switch (m_eAuthenticated)
    {
    case TGT_SSH_AUTH_STATE_USER_ACCEPT:
        nRet = TgtAuthenticateAcceptInput(szWriteData, nBytes, m_szUsername, TGT_SSH_AUTH_STATE_PASS_PROMPT);
        break;
    case TGT_SSH_AUTH_STATE_PASS_ACCEPT:
        nRet = TgtAuthenticateAcceptInput(szWriteData, nBytes, m_szPassword, TGT_SSH_AUTH_STATE_PASS_AUTHENTICATE);
        break;
    }
    return nRet;
}

int TgtSshIntf::TgtRead(char *szReadData, int nMaxBytes)
{
    struct timeval tv;
    int nNumBytes = 0;
    ssh_channel chans[] = {m_sshChannel, 0};

    if (m_eAuthenticated != TGT_SSH_AUTH_STATE_DONE)
    {
        nNumBytes = TgtAuthenticatePrompt(szReadData, nMaxBytes);
    }
    else
    {
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        if (channel_select(chans, NULL, NULL, &tv) > 0)
        {
            nNumBytes = channel_read_nonblocking(m_sshChannel, szReadData, nMaxBytes, false);
        }
    }
    m_nTotalRx += nNumBytes;
    return nNumBytes;
}

int TgtSshIntf::TgtWrite(char *szWriteData, int nBytes)
{
    int nNumBytes = 0;
    if (m_eAuthenticated != TGT_SSH_AUTH_STATE_DONE)
    {
        nNumBytes = TgtAuthenticateAccept(szWriteData, nBytes);
    }
    else
    {
        nNumBytes = channel_write(m_sshChannel, szWriteData, nBytes);
    }
    m_nTotalTx += nNumBytes;
    return nNumBytes;
}

bool TgtSshIntf::TgtConnected()
{
    return true;
}

void TgtSshIntf::TgtGetTitle(char *szTitle)
{
    if (_stricmp(m_sTgtConnection.m_szServerName, m_sTgtConnection.m_szDescription) == 0)
    {
        sprintf(szTitle, "SSH %s", 
            m_sTgtConnection.m_szDescription);
    }
    else
    {
        sprintf(szTitle, "SSH %s - %s", 
            m_sTgtConnection.m_szDescription, 
            m_sTgtConnection.m_szServerName);
    }
}

/******************************************************************************
**
**  Telnet
**
******************************************************************************/

TgtTelnetIntf::TgtTelnetIntf()
{
    m_nSocket = -1;
    m_nState = TELNET_STATE_DATA;
    m_nCommand = TELNET_CMD_SB;
    m_bEcho = true;
    m_bConnected = false;
}

TgtTelnetIntf::~TgtTelnetIntf()
{
    m_bConnected = false;
}

char * TgtTelnetIntf::TgtMakeConnection()
{
    char *pRet;
    struct hostent  *pHostEnt;
    struct protoent *pProtEnt;
    struct sockaddr_in sin;
    unsigned long nNonBlocking = 1;

    pRet = NULL;
    memset((char *)&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons((u_short)m_sTgtConnection.m_nPort);
    pHostEnt = gethostbyname(m_sTgtConnection.m_szServerName);
    if (pHostEnt != 0)
    {
        memcpy((char *)&sin.sin_addr, pHostEnt->h_addr, pHostEnt->h_length);
    }
    else
    {
        sin.sin_addr.s_addr = inet_addr(m_sTgtConnection.m_szServerName);
        if (sin.sin_addr.s_addr == INADDR_NONE)
        {
            pRet = "Unable to find server";
        }
    }
    pProtEnt = getprotobyname("tcp");
    if (pProtEnt == 0)
    {
        pRet = "TCP failure";
    }
    else
    {
        m_nSocket = (int)socket(PF_INET, SOCK_STREAM, pProtEnt->p_proto);
        if (m_nSocket < 0)
        {
            pRet = "Unable to open socket";
        }
        else
        {
            if (connect(m_nSocket, (struct sockaddr *)&sin, sizeof(sin)) == SOCKET_ERROR)
            {
                pRet = "Unable to connect to server";
            }
            else
            {
                if (ioctlsocket(m_nSocket, FIONBIO, &nNonBlocking) == SOCKET_ERROR)
                {
                    pRet = "Socket failure";
                }
                else
                {
                    m_bConnected = true;
                }
            }
        }
    }
    return pRet;
}

int TgtTelnetIntf::TgtDisconnect()
{
    m_bConnected = false;
    if (m_nSocket > 0)
    {
        closesocket(m_nSocket);
    }
    return 0;
}

int TgtTelnetIntf::TgtTelnetData(unsigned char cTelnetRx, char* cReadData)
{
    int nRet = 0;
    switch(cTelnetRx)
    {
    case TELNET_CMD_IAC:
        m_nState = TELNET_STATE_COMMAND;
        break;
    default:
        if (m_bEcho)
        {
            *cReadData = cTelnetRx;
        }
        nRet = 1;
        break;
    }
    return nRet;
}

int TgtTelnetIntf::TgtTelnetCommand(eTelnetCommand cTelnetRx)
{
    m_nState = TELNET_STATE_DATA;
    m_nCommand = cTelnetRx;
    switch (cTelnetRx)
    {
    case TELNET_CMD_IAC:
    case TELNET_CMD_SE:
    case TELNET_CMD_NOP:
    case TELNET_CMD_DM:
    case TELNET_CMD_BRK:
    case TELNET_CMD_IP:
    case TELNET_CMD_AO:
    case TELNET_CMD_AYT:
    case TELNET_CMD_EL:
    case TELNET_CMD_GA:
    case TELNET_CMD_EC:
        break;
    case TELNET_CMD_SB:
    case TELNET_CMD_WILL:
    case TELNET_CMD_WONT:
    case TELNET_CMD_DO:
    case TELNET_CMD_DONT:
        m_nState = TELNET_STATE_OPTION;
        break;
    }
    return 0;
}

int TgtTelnetIntf::TgtSendCommand(eTelnetCommand eCmd, eTelnetOption eOpt)
{
    unsigned char sBuffer[3];
    sBuffer[0] = TELNET_CMD_IAC;
    sBuffer[1] = eCmd;
    sBuffer[2] = eOpt;
    return send(m_nSocket, (char*)sBuffer, sizeof(sBuffer), 0);
}

int TgtTelnetIntf::TgtConfirm(eTelnetOption eOpt)
{
    switch(m_nCommand)
    {
    case TELNET_CMD_WILL:
        TgtSendCommand(TELNET_CMD_DO, eOpt);
        break;
    case TELNET_CMD_WONT:
        TgtSendCommand(TELNET_CMD_DONT, eOpt);
        break;
    case TELNET_CMD_DO:
        TgtSendCommand(TELNET_CMD_WILL, eOpt);
        break;
    case TELNET_CMD_DONT:
        TgtSendCommand(TELNET_CMD_WONT, eOpt);
        break;
    }
    return 0;
}

int TgtTelnetIntf::TgtDeny(eTelnetOption eOpt)
{
    switch(m_nCommand)
    {
    case TELNET_CMD_WILL:
        TgtSendCommand(TELNET_CMD_DONT, eOpt);
        break;
    case TELNET_CMD_WONT:
        TgtSendCommand(TELNET_CMD_DO, eOpt);
        break;
    case TELNET_CMD_DO:
        TgtSendCommand(TELNET_CMD_WONT, eOpt);
        break;
    case TELNET_CMD_DONT:
        TgtSendCommand(TELNET_CMD_WILL, eOpt);
        break;
    }
    return 0;
}

int TgtTelnetIntf::TgtProcessTerm()
{
    int nReadIndex = 0;
    char sBuffer[] = 
    {
        (char)TELNET_CMD_IAC,
        (char)TELNET_CMD_SB,
        (char)TELNET_OPT_TERM,
        0,
        'v','t','3','2','0',
        (char)TELNET_CMD_IAC,
        (char)TELNET_CMD_SE
    };
    switch(m_nCommand)
    {
    case TELNET_CMD_SB:
        send(m_nSocket, (char*)sBuffer, sizeof(sBuffer), 0);
        break;
    case TELNET_CMD_WILL:
        TgtDeny(TELNET_OPT_TERM);
        break;
    case TELNET_CMD_WONT:
        break;
    case TELNET_CMD_DO:
        TgtConfirm(TELNET_OPT_TERM);
        break;
    case TELNET_CMD_DONT:
        break;
    }
    return nReadIndex;
}

int TgtTelnetIntf::TgtProcessEcho()
{
    switch(m_nCommand)
    {
    case TELNET_CMD_SB:
        break;
    case TELNET_CMD_WILL:
        m_bEcho = true;
        TgtConfirm(TELNET_OPT_ECHO);
        break;
    case TELNET_CMD_WONT:
        m_bEcho = false;
        TgtConfirm(TELNET_OPT_ECHO);
        break;
    case TELNET_CMD_DO:
        TgtDeny(TELNET_OPT_ECHO);
        break;
    case TELNET_CMD_DONT:
        TgtConfirm(TELNET_OPT_ECHO);
        break;
    }
    return 0;
}

int TgtTelnetIntf::TgtProcessUnknownOption(eTelnetOption eOpt)
{
    switch(m_nCommand)
    {
    case TELNET_CMD_WILL:
    case TELNET_CMD_DO:
        TgtDeny(eOpt);
        break;
    }
    return 0;
}

int TgtTelnetIntf::TgtTelnetOption(eTelnetOption eOpt)
{
    int nReadIndex = 0;
    m_nState = TELNET_STATE_DATA;
    switch (eOpt)
    {
    case TELNET_OPT_ECHO   :
        nReadIndex = TgtProcessEcho();
        break;
    case TELNET_OPT_TERM   :
        nReadIndex = TgtProcessTerm();
        break;
    case TELNET_OPT_SUPP   :
    case TELNET_OPT_BIN    :
    case TELNET_OPT_RECN   :
    case TELNET_OPT_APRX   :
    case TELNET_OPT_STAT   :
    case TELNET_OPT_TIM    :
    case TELNET_OPT_REM    :
    case TELNET_OPT_OLW    :
    case TELNET_OPT_OPS    :
    case TELNET_OPT_OCRD   :
    case TELNET_OPT_OHT    :
    case TELNET_OPT_OHTD   :
    case TELNET_OPT_OFD    :
    case TELNET_OPT_OVT    :
    case TELNET_OPT_OVTD   :
    case TELNET_OPT_OLD    :
    case TELNET_OPT_EXT    :
    case TELNET_OPT_LOGO   :
    case TELNET_OPT_BYTE   :
    case TELNET_OPT_DATA   :
    case TELNET_OPT_SUP    :
    case TELNET_OPT_SUPO   :
    case TELNET_OPT_SNDL   :
    case TELNET_OPT_EOR    :
    case TELNET_OPT_TACACS :
    case TELNET_OPT_OM     :
    case TELNET_OPT_TLN    :
    case TELNET_OPT_3270   :
    case TELNET_OPT_X3     :
    case TELNET_OPT_NAWS   :
    case TELNET_OPT_TS     :
    case TELNET_OPT_RFC    :
    case TELNET_OPT_LINE   :
    case TELNET_OPT_XDL    :
    case TELNET_OPT_ENVIR  :
    case TELNET_OPT_AUTH   :
    case TELNET_OPT_NENVIR :
    case TELNET_OPT_EXTOP  :
        TgtProcessUnknownOption(eOpt);
        break;
    }
    return nReadIndex;
}

int TgtTelnetIntf::TgtTelnet(char *sTelnetRx, int nNumBytes, char *szReadData)
{
    int nRxIndex;
    int nReadIndex = 0;
    for (nRxIndex = 0; nRxIndex < nNumBytes; nRxIndex++)
    {
        switch (m_nState)
        {
        case TELNET_STATE_DATA:
            nReadIndex += TgtTelnetData(sTelnetRx[nRxIndex], &szReadData[nReadIndex]);
            break;
        case TELNET_STATE_COMMAND:
            TgtTelnetCommand((eTelnetCommand)(sTelnetRx[nRxIndex] & 0xFF));
            break;
        case TELNET_STATE_OPTION:
            TgtTelnetOption((eTelnetOption)(sTelnetRx[nRxIndex] & 0xFF));
            if (m_nCommand == TELNET_CMD_SB)
            {
                while (((sTelnetRx[nRxIndex + 1] & 0xFF) != TELNET_CMD_IAC) && (nRxIndex < nNumBytes))
                {
                    nRxIndex++;
                }
            }
            break;
        }
    }
    return nReadIndex;
}

int TgtTelnetIntf::TgtRead(char *szReadData, int nMaxBytes)
{
    int nNumBytes;

    nNumBytes = recv(m_nSocket, m_sTelnetRx, sizeof(m_sTelnetRx), 0);
    if (nNumBytes > 0)
    {
        m_nTotalRx += nNumBytes;
    }
    else if ((nNumBytes == SOCKET_ERROR) && (WSAGetLastError() != WSAEWOULDBLOCK))
    {
        void DebugOutput(const char *szFormat, ...);

        DebugOutput("recv error: %i\n", WSAGetLastError());
        m_bConnected = false;
    }
    return TgtTelnet(m_sTelnetRx, nNumBytes, szReadData);
}

int TgtTelnetIntf::TgtWrite(char *szWriteData, int nBytes)
{
    int nNumBytes;
    nNumBytes = send(m_nSocket, szWriteData, nBytes, 0);
    if (nNumBytes > 0)
    {
        m_nTotalTx += nNumBytes;
    }
    else if (nNumBytes == SOCKET_ERROR)
    {
        void DebugOutput(const char *szFormat, ...);

        DebugOutput("send error: %i\n", WSAGetLastError());
        m_bConnected = false;
    }
    return nNumBytes;
}

bool TgtTelnetIntf::TgtConnected()
{
    return m_bConnected;
}

void TgtTelnetIntf::TgtGetTitle(char *szTitle)
{
    if (_stricmp(m_sTgtConnection.m_szServerName, m_sTgtConnection.m_szDescription) == 0)
    {
        sprintf(szTitle, "Telnet %s %i", 
            m_sTgtConnection.m_szDescription,
            m_sTgtConnection.m_nPort);
    }
    else
    {
        sprintf(szTitle, "Telnet %s - %s %i", 
            m_sTgtConnection.m_szDescription, 
            m_sTgtConnection.m_szServerName,
            m_sTgtConnection.m_nPort);
    }
}

/******************************************************************************
**
**  Serial
**
******************************************************************************/

TgtSerialIntf::TgtSerialIntf ()
{
    InitializeCriticalSection(&m_csInProtector);
    InitializeCriticalSection(&m_csOutProtector);
    m_hSerial = 0;
    m_hThreadTerm = 0;
    m_hOutput = 0;
}

TgtSerialIntf::~TgtSerialIntf ()
{
	DeleteCriticalSection (&m_csInProtector);
    DeleteCriticalSection (&m_csOutProtector);
}

char * TgtSerialIntf::TgtSetupPort()
{
    char *pRet;
    DCB dDcb;

    pRet = NULL;

    memset(&dDcb, 0, sizeof(DCB));
    dDcb.DCBlength = sizeof(DCB);
    if (!GetCommState(m_hSerial, &dDcb))
    {
        pRet = "GetCommState failed";
    }
    else
    {
        dDcb.BaudRate = m_sTgtConnection.m_dwBaudRate;
        dDcb.ByteSize = m_sTgtConnection.m_byByteSize;
        dDcb.Parity = m_sTgtConnection.m_byParity;
        dDcb.StopBits = m_sTgtConnection.m_byStopBits;
        dDcb.fDsrSensitivity = 0;
        dDcb.fDtrControl = DTR_CONTROL_ENABLE;
        dDcb.fOutxDsrFlow = 0;
        if (!SetCommState(m_hSerial, &dDcb))
        {
            pRet = "SetCommState failed";
        }
    }
    return pRet;
}

char * TgtSerialIntf::TgtMakeConnection()
{
    char *pRet;
    COMMTIMEOUTS cTimeOuts;

    pRet = NULL;

    m_hSerial = CreateFile(m_sTgtConnection.m_szPortName,  
        GENERIC_READ | GENERIC_WRITE, 
        0, 0, OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED, 0);
    if (m_hSerial == INVALID_HANDLE_VALUE)
    {
        pRet = "Unable to open serial port";
    }
    else if (!SetCommMask(m_hSerial, EV_RXCHAR | EV_TXEMPTY))
    {
        pRet = "SetCommMask failed";
    }
    else 
    {
        pRet = TgtSetupPort();
        if (pRet == NULL)
        {
            cTimeOuts.ReadIntervalTimeout = MAXDWORD; 
            cTimeOuts.ReadTotalTimeoutMultiplier = 0;
            cTimeOuts.ReadTotalTimeoutConstant = 0;
            cTimeOuts.WriteTotalTimeoutMultiplier = 0;
            cTimeOuts.WriteTotalTimeoutConstant = 0;

            if (!SetCommTimeouts(m_hSerial, &cTimeOuts))
            {
                pRet = "SetCommTimeouts failed";
            }
            else
            {
                m_hOutput = CreateEvent(0,0,0,0);
                m_hThreadTerm = CreateEvent(0,0,0,0);
                m_hSerialMonitor = (HANDLE)_beginthread(&TgtSerialIntf::TgtSerialMonitor, 0,
                    (LPVOID)this);
            }
        }
    }
    return pRet;
}

void TgtSerialIntf::TgtSerialMonitor(void *arg)
{
	TgtSerialIntf* _this = (TgtSerialIntf*) arg;
    HANDLE arHandles[3];
    OVERLAPPED ov;
    DWORD dwWait;
    DWORD dwEventMask = 0;

    arHandles[0] = _this->m_hThreadTerm;

    memset(&ov, 0, sizeof(ov));
	ov.hEvent = CreateEvent(0, true, 0, 0);

    while (1)
    {
        WaitCommEvent(_this->m_hSerial, &dwEventMask, &ov);
        arHandles[1] = ov.hEvent;
        arHandles[2] = _this->m_hOutput;
        dwWait = WaitForMultipleObjects(3, arHandles, FALSE, INFINITE);
        switch (dwWait)
        {
        case WAIT_OBJECT_0:
            _endthread();
            return;
        case WAIT_OBJECT_0 + 1:
            _this->TgtReadFromPort();
            ResetEvent(ov.hEvent);
            break;
        case WAIT_OBJECT_0 + 2:
            _this->TgtSendToPort();
            ResetEvent(_this->m_hOutput);
            break;
        }
    }
    CloseHandle(ov.hEvent);
}

void TgtSerialIntf::TgtWritePortData(char *szData, DWORD nBytes)
{
    DWORD dwTotalBytesWrote;
    DWORD dwBytesWrote;
    OVERLAPPED ovWrite;
    DWORD dwMask;
    BOOL bRet;
    HANDLE arHandles[2];
    DWORD dwErrors;

    dwTotalBytesWrote = 0;
    do
    {
        GetCommMask(m_hSerial, &dwMask);

        if (dwMask & EV_TXEMPTY)
        {
            memset(&ovWrite, 0, sizeof(ovWrite));
            ovWrite.hEvent = CreateEvent(0, true, 0, 0);
            bRet = WriteFile(m_hSerial, szData, nBytes, &dwBytesWrote, &ovWrite);
            if (!bRet)
            {
                arHandles[0] = m_hThreadTerm;
                arHandles[1] = ovWrite.hEvent;
                if (GetLastError() == ERROR_IO_PENDING)
                {
                    switch (WaitForMultipleObjects(2, arHandles, FALSE, 1000))
                    {
                    case WAIT_OBJECT_0:
                        _endthread();
                        break;
                    case WAIT_OBJECT_0 + 1:
                        GetOverlappedResult(m_hSerial, &ovWrite, &dwBytesWrote, FALSE);
                        dwTotalBytesWrote += dwBytesWrote;
                        break;
                    }
                }
                else
                {
                    ClearCommError(m_hSerial, &dwErrors, NULL);
                    dwBytesWrote = 0;
                }
            }
            else
            {
                dwTotalBytesWrote += dwBytesWrote;
            }
            CloseHandle(ovWrite.hEvent);
        }
        else
        {
            Sleep(1);
        }
    } while (dwTotalBytesWrote < nBytes);
}

void TgtSerialIntf::TgtSendToPort()
{
    char szData[64];
    DWORD nBytes = 0;
    do
    {
        EnterCriticalSection(&m_csOutProtector);
        nBytes = Vec2Char(szData, &m_dataOutgoing, sizeof(szData));
        LeaveCriticalSection(&m_csOutProtector);
        TgtWritePortData(szData, nBytes);
        m_nTotalTx += nBytes;
    } while (m_dataOutgoing.size() > 0);
}

void TgtSerialIntf::TgtReadFromPort()
{
    OVERLAPPED ovRead;
    DWORD dwBytesRead;
    DWORD dwErrors;
    char buf;
    BOOL bRet;

    memset(&ovRead, 0, sizeof(ovRead));
    ovRead.hEvent = CreateEvent(0, true, 0, 0);

    do
    {
        ResetEvent(ovRead.hEvent);
        bRet = ReadFile(m_hSerial, &buf, sizeof(buf), &dwBytesRead, &ovRead);
        if (dwBytesRead > 0)
        {
            EnterCriticalSection(&m_csInProtector);
            m_dataIncoming.push_back(buf);
            LeaveCriticalSection(&m_csInProtector);
        }
        m_nTotalRx += dwBytesRead;
    } while ((bRet) && (dwBytesRead));
    if (!bRet)
    {
        ClearCommError(m_hSerial, &dwErrors, NULL);
    }
	CloseHandle(ovRead.hEvent);
}

int TgtSerialIntf::TgtDisconnect()
{
    if (m_hThreadTerm)
    {
        SignalObjectAndWait(m_hThreadTerm, m_hSerialMonitor, 1000, FALSE);
        CloseHandle(m_hThreadTerm);
    }
    if (m_hOutput)
    {
        CloseHandle(m_hOutput);
    }
    if (m_hSerial)
    {
        CloseHandle(m_hSerial);
    }
    return 0;
}

int TgtSerialIntf::TgtRead(char *szReadData, int nMaxBytes)
{
    int nRet = 0;

    EnterCriticalSection(&m_csInProtector);
    nRet = Vec2Char(szReadData, &m_dataIncoming, nMaxBytes);
    LeaveCriticalSection(&m_csInProtector);

    return nRet;
}

int TgtSerialIntf::TgtWrite(char *szWriteData, int nBytes)
{
    int nIndex;
    EnterCriticalSection(&m_csOutProtector);
    for (nIndex = 0; nIndex < nBytes; nIndex++)
    {
        m_dataOutgoing.push_back(szWriteData[nIndex]);
    }
    LeaveCriticalSection(&m_csOutProtector);
    SetEvent(m_hOutput);
    return 0;
}

bool TgtSerialIntf::TgtConnected()
{
    return true;
}

void TgtSerialIntf::TgtGetTitle(char *szTitle)
{
    char parity;
    switch (m_sTgtConnection.m_byParity)
    {
    case NOPARITY:
        parity = 'N';
        break;
    case ODDPARITY:
        parity = 'O';
        break;
    case EVENPARITY:
        parity = 'E';
        break;
    case MARKPARITY:
        parity = 'M';
        break;
    case SPACEPARITY:
        parity = 'S';
        break;
    }
    sprintf(szTitle, "SERIAL %s %i %c-%i-%i", 
        m_sTgtConnection.m_szPortName,
        m_sTgtConnection.m_dwBaudRate,
        parity, m_sTgtConnection.m_byByteSize,
        m_sTgtConnection.m_byStopBits + 1);
}


/******************************************************************************
**
**  File
**
******************************************************************************/


TgtFileIntf::TgtFileIntf ()
{
    m_cnt = 0;
}

TgtFileIntf::~TgtFileIntf ()
{
}

char * TgtFileIntf::TgtMakeConnection()
{
    static char szError[80];
    m_fInput = fopen(m_sTgtConnection.m_szFileName, "rb");
    if (m_fInput)
    {
        return NULL;
    }
    sprintf(szError, "Unable to open file %s", m_sTgtConnection.m_szFileName);
    return szError;
}

int TgtFileIntf::TgtDisconnect()
{
    return 0;
}

int TgtFileIntf::TgtRead(char *szReadData, int nMaxBytes)
{
    int nLen;
    if (fread(&nLen, sizeof(nLen), 1, m_fInput) == 1)
    {
        return fread(szReadData, sizeof(char), nLen, m_fInput);
    }
    return 0;
}

int TgtFileIntf::TgtWrite(char *szWriteData, int nBytes)
{
    return 0;
}

bool TgtFileIntf::TgtConnected()
{
    return true;
}

void TgtFileIntf::TgtGetTitle(char *szTitle)
{
    strcpy(szTitle, m_sTgtConnection.m_szFileName);
}
