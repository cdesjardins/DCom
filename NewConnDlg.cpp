// NewConnDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DCom.h"
#include "DComDoc.h"
#include "NewConnDlg.h"

#define DCOM_CONNECTION_SECTION   "Connection"
#define DCOM_CONNECTION_DATA      "%i - Data v0"
#define DCOM_CONNECTION_INDEX     "Index"
#define DCOM_MAX_CONNECTIONS      10
// CNewConnDlg dialog

IMPLEMENT_DYNAMIC(CNewConnDlg, CSizeDialog)

CNewConnDlg::CNewConnDlg(CWnd* pParent /*=NULL*/)
	: CSizeDialog(CNewConnDlg::IDD, pParent)
{
    memset(&m_sDialingDirSelection, 0, sizeof(m_sDialingDirSelection));
}

CNewConnDlg::~CNewConnDlg()
{
}

void CNewConnDlg::DoDataExchange(CDataExchange* pDX)
{
    CSizeDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMMPORT_COMBO, m_commCombo);
    DDX_Control(pDX, IDC_PARITY_COMBO, m_parityCombo);
    DDX_Control(pDX, IDC_BYTESIZE_COMBO, m_byteSizeCombo);
    DDX_Control(pDX, IDC_STOPBITS_COMBO, m_stopBitsCombo);
    DDX_Control(pDX, IDC_SSH_RDO, m_sshRdo);
    DDX_Control(pDX, IDC_TELNET_RDO, m_telnetRdo);
    DDX_Control(pDX, IDC_SERIAL_RDO, m_serialRdo);
    DDX_Control(pDX, IDC_HOSTNAME_COMBO, m_hostNameCombo);
    DDX_Control(pDX, IDC_PORT_EDIT, m_portEdit);
    DDX_Control(pDX, IDC_BAUD_COMBO, m_baudCombo);
    DDX_Control(pDX, IDC_DEBUG_RDO, m_debugRdo);
}


BEGIN_MESSAGE_MAP(CNewConnDlg, CSizeDialog)
    ON_BN_CLICKED(IDC_SSH_RDO, &CNewConnDlg::OnBnClickedSshRdo)
    ON_BN_CLICKED(IDC_TELNET_RDO, &CNewConnDlg::OnBnClickedTelnetRdo)
    ON_BN_CLICKED(IDC_SERIAL_RDO, &CNewConnDlg::OnBnClickedSerialRdo)
    ON_BN_CLICKED(IDOK, &CNewConnDlg::OnBnClickedOk)
    ON_CBN_SELCHANGE(IDC_HOSTNAME_COMBO, &CNewConnDlg::OnCbnSelchangeHostnameCombo)
    ON_WM_DESTROY()
    ON_BN_CLICKED(IDC_DIALING_DIR_BUTTON, &CNewConnDlg::OnBnClickedDialingDirButton)
    ON_MESSAGE(WM_DIALING_DIR_OK, OnDialingDirOK)
    ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CNewConnDlg)
END_EASYSIZE_MAP

// CNewConnDlg message handlers

void CNewConnDlg::DetectComPorts()
{
    HANDLE hSerial;
    char szCom[32];
    int i;

    m_commCombo.ResetContent();
    for (i = 0; i < 16; i++)
    {
        sprintf(szCom, "COM%i", i);
        hSerial = CreateFile(szCom,
            GENERIC_READ | GENERIC_WRITE, 
            0, 0, OPEN_EXISTING,
            0, 0);
        if (hSerial != INVALID_HANDLE_VALUE)
        {
            m_commCombo.AddString(szCom);
            CloseHandle(hSerial);
        }
    }
    m_commCombo.SetCurSel(0);
}

char *CNewConnDlg::GetSectionName()
{
    return "New Connection";
}

BOOL CNewConnDlg::OnInitDialog()
{
    CSizeDialog::OnInitDialog();
#ifndef CATCH_DEBUG
    m_debugRdo.ShowWindow(FALSE);
#endif
    m_parityCombo.SetCurSel(0);
    m_stopBitsCombo.SetCurSel(0);
    m_byteSizeCombo.SetCurSel(0);
    m_baudCombo.SetCurSel(0);
    
    m_sshRdo.SetCheck(1);
    m_portEdit.SetWindowText("22");
    OnBnClickedSshRdo();

    ParseConnectionData();

    return TRUE;
}

void CNewConnDlg::OnBnClickedSshRdo()
{
    CString szPort;
    m_commCombo.EnableWindow(FALSE);
    m_parityCombo.EnableWindow(FALSE);
    m_stopBitsCombo.EnableWindow(FALSE);
    m_byteSizeCombo.EnableWindow(FALSE);
    m_baudCombo.EnableWindow(FALSE);
    
    m_hostNameCombo.EnableWindow(TRUE);
    m_portEdit.EnableWindow(TRUE);

    m_portEdit.GetWindowText(szPort);
    if (szPort.Compare("23") == 0)
    {
        m_portEdit.SetWindowText("22");
    }
}

void CNewConnDlg::OnBnClickedTelnetRdo()
{
    CString szPort;
    m_commCombo.EnableWindow(FALSE);
    m_parityCombo.EnableWindow(FALSE);
    m_stopBitsCombo.EnableWindow(FALSE);
    m_byteSizeCombo.EnableWindow(FALSE);
    m_baudCombo.EnableWindow(FALSE);
    
    m_hostNameCombo.EnableWindow(TRUE);
    m_portEdit.EnableWindow(TRUE);

    m_portEdit.GetWindowText(szPort);
    if (szPort.Compare("22") == 0)
    {
        m_portEdit.SetWindowText("23");
    }
}

void CNewConnDlg::OnBnClickedSerialRdo()
{
    m_commCombo.EnableWindow(TRUE);
    m_parityCombo.EnableWindow(TRUE);
    m_stopBitsCombo.EnableWindow(TRUE);
    m_byteSizeCombo.EnableWindow(TRUE);
    m_baudCombo.EnableWindow(TRUE);
    
    m_hostNameCombo.EnableWindow(FALSE);
    m_portEdit.EnableWindow(FALSE);
}

void CNewConnDlg::OnBnClickedOk()
{
    bool bRet = false;
    POSITION dtPos = AfxGetApp()->GetFirstDocTemplatePosition();
    CMyMultiDocTemplate *pDocTemplate;

    if (dtPos != NULL)
    {
        pDocTemplate = (CMyMultiDocTemplate*)AfxGetApp()->GetNextDocTemplate(dtPos);
        if (pDocTemplate)
        {
            bRet = GetDataFromControls(&m_sDialingDir);
            if (bRet)
            {
                if (pDocTemplate->OpenDocumentFile(&m_sDialingDir))
                {
                    AddNewRegistryData(&m_sDialingDir);
                    OnOK();
                }
            }
        }
    }
}

bool CNewConnDlg::GetDataFromControls(SDialingDir *sDialingDir)
{
    CString szTmp;
    bool bRet = true;

    memset(sDialingDir, 0, sizeof(SDialingDir));
    
    if (m_serialRdo.GetCheck() == BST_CHECKED)
    {
        sDialingDir->nType = DCOM_DD_SERIAL;
        return GetDataFromSerialControls(sDialingDir);
    }
    else if (m_sshRdo.GetCheck() == BST_CHECKED)
    {
        sDialingDir->nType = DCOM_DD_SSH;
    }
    else if (m_telnetRdo.GetCheck() == BST_CHECKED)
    {
        sDialingDir->nType = DCOM_DD_TELNET;
    }
    else 
    {
        sDialingDir->nType = DCOM_DD_DEBUG;
    }
    
    m_hostNameCombo.GetWindowText(szTmp);
    if (szTmp.GetLength() == 0)
    {
        bRet = false;
        AfxMessageBox("Invalid host name");
    }
    else
    {
        if (strcmp(szTmp.GetBuffer(), m_sDialingDirSelection.szDescription) == 0)
        {
            strncpy(sDialingDir->szDescription, m_sDialingDirSelection.szDescription, sizeof(sDialingDir->szDescription));
            strncpy(sDialingDir->szAddress, m_sDialingDirSelection.szAddress, sizeof(sDialingDir->szAddress));
        }
        else
        {
            strncpy(sDialingDir->szAddress, szTmp.GetBuffer(), sizeof(sDialingDir->szAddress));
        }

        m_portEdit.GetWindowText(szTmp);
        if (szTmp.GetLength() == 0)
        {
            bRet = false;
            AfxMessageBox("Invalid port");
        }
        else
        {
            sDialingDir->nPort = atoi(szTmp.GetBuffer());
        }
    }
    return bRet;
}

bool CNewConnDlg::GetDataFromSerialControls(SDialingDir *sDialingDir)
{
    char szTmp[32];
    int nTmp;
    
    /* COMn */
    m_commCombo.GetLBText(m_commCombo.GetCurSel(), szTmp);
    sprintf(sDialingDir->szAddress, "%s", szTmp);

    /* BAUD */
    m_baudCombo.GetLBText(m_baudCombo.GetCurSel(), szTmp);
    sscanf(szTmp, "%i", &nTmp);
    sDialingDir->m_dwBaudRate = nTmp;

    /* Pairity */
    m_parityCombo.GetLBText(m_parityCombo.GetCurSel(), szTmp);
    switch (szTmp[0])
    {
    case 'N':
        sDialingDir->m_byParity = NOPARITY;
        break;
    case 'O':
        sDialingDir->m_byParity = ODDPARITY;
        break;
    case 'E':
        sDialingDir->m_byParity = EVENPARITY;
        break;
    case 'M':
        sDialingDir->m_byParity = MARKPARITY;
        break;
    case 'S':
        sDialingDir->m_byParity = SPACEPARITY;
        break;
    }

    /* Byte Size */
    m_byteSizeCombo.GetLBText(m_byteSizeCombo.GetCurSel(), szTmp);
    sscanf(szTmp, "%i", &nTmp);
    sDialingDir->m_byByteSize = (unsigned char)nTmp;

    /* Stop Bits */
    m_stopBitsCombo.GetLBText(m_stopBitsCombo.GetCurSel(), szTmp);
    switch(szTmp[0])
    {
    case '1':
        sDialingDir->m_byStopBits = ONESTOPBIT;
        break;
    case '2':
        sDialingDir->m_byStopBits = TWOSTOPBITS;
        break;
    }
    return true;
}

void CNewConnDlg::AddNewRegistryData(SDialingDir *sNewDialingDir)
{
    UINT nIndex;
    UINT nNumData;
    char szBuf[32];
    SDialingDir *sDialingDir;

    nNumData = 0;
    if (sNewDialingDir->nType == DCOM_DD_SERIAL)
    {
        return;
    }
    while (nNumData < DCOM_MAX_CONNECTIONS)
    {
        sDialingDir = GetDataEntryFromRegistry(nNumData++);
        if (sDialingDir)
        {
            /* Dont put duplicate registry entries out there */
            if (memcmp(sDialingDir, sNewDialingDir, sizeof(SDialingDir)) == 0)
            {
                delete sDialingDir;
                return;
            }
            delete sDialingDir;
        }
        else
        {
            break;
        }
    }
    nIndex = AfxGetApp()->GetProfileInt(DCOM_DIALINGDIR_SECTION, DCOM_CONNECTION_INDEX, 0);
    sprintf(szBuf, DCOM_CONNECTION_DATA, nIndex);
    nIndex = (nIndex + 1) % DCOM_MAX_CONNECTIONS;
    AfxGetApp()->WriteProfileBinary(DCOM_DIALINGDIR_SECTION, szBuf, (LPBYTE)sNewDialingDir, sizeof(SDialingDir));
    AfxGetApp()->WriteProfileInt(DCOM_DIALINGDIR_SECTION, DCOM_CONNECTION_INDEX, nIndex);
    UpdateCombos(sNewDialingDir);
}

SDialingDir *CNewConnDlg::GetDataEntryFromRegistry(int nRegIndex)
{
    CWinApp *pApp;
    SDialingDir *sDialingDir;
    UINT nLen;
    CString szReg;

    pApp = AfxGetApp();
    szReg.Format(DCOM_CONNECTION_DATA, nRegIndex);
    if (pApp->GetProfileBinary(DCOM_DIALINGDIR_SECTION, szReg, (LPBYTE*)&sDialingDir, &nLen) == TRUE)
    {
        if (nLen <= sizeof(SDialingDir))
        {
            memset(sDialingDir->reserved0, 0, sizeof(sDialingDir->reserved0));
            memset(sDialingDir->reserved1, 0, sizeof(sDialingDir->reserved1));
            return sDialingDir;
        }
        else
        {
            delete sDialingDir;
        }
    }
    return NULL;
}

void CNewConnDlg::ParseConnectionData()
{
    UINT nRegIndex;
    CString szData;
    SDialingDir *sDialingDir;
    CWinApp *pApp;

    pApp = AfxGetApp();
    nRegIndex = 0;
    while (nRegIndex < DCOM_MAX_CONNECTIONS)
    {
        sDialingDir = GetDataEntryFromRegistry(nRegIndex++);
        if (sDialingDir)
        {
            UpdateCombos(sDialingDir);
            delete sDialingDir;
        }
        else
        {
            break;
        }
    }
}

void CNewConnDlg::UpdateCombos(SDialingDir *sDialingDir)
{
    int nIndex;

    nIndex = m_hostNameCombo.FindStringExact(-1, sDialingDir->szAddress);
    if (nIndex == CB_ERR)
    {
        nIndex = m_hostNameCombo.FindStringExact(-1, sDialingDir->szDescription);
    }
    if ((nIndex == CB_ERR) || (memcmp(&m_sDialingDirVec[nIndex], sDialingDir, sizeof(SDialingDir)) != 0))
    {
        m_sDialingDirVec.push_back(*sDialingDir);
        if (strlen(sDialingDir->szDescription) > 0)
        {
            nIndex = m_hostNameCombo.AddString(sDialingDir->szDescription);
        }
        else
        {
            nIndex = m_hostNameCombo.AddString(sDialingDir->szAddress);
        }
        if (sDialingDir->nType == DCOM_DD_SSH)
        {
            m_hostNameCombo.SetBitmap(nIndex, IDB_SSH);
        }
        else
        {
            m_hostNameCombo.SetBitmap(nIndex, IDB_TELNET);
        }
        m_hostNameCombo.SetItemUserData(nIndex, m_sDialingDirVec.size() - 1);
    }
}

void CNewConnDlg::OnCbnSelchangeHostnameCombo()
{
    int nIndex;
    int nSel;
    nSel = m_hostNameCombo.GetCurSel();
    if (nSel != CB_ERR)
    {
        nIndex = m_hostNameCombo.GetItemUserData(nSel);
        PopulateControls(&m_sDialingDirVec[nIndex]);
        memcpy(&m_sDialingDirSelection, &m_sDialingDirVec[nIndex], sizeof(SDialingDir));
    }
}

void CNewConnDlg::PopulateControls(SDialingDir *sDialingDir)
{
    char szPort[32];
    sprintf(szPort, "%i", sDialingDir->nPort);
    m_portEdit.SetWindowText(szPort);
    if (sDialingDir->nType == DCOM_DD_TELNET)
    {
        OnBnClickedTelnetRdo();
        m_sshRdo.SetCheck(0);
        m_telnetRdo.SetCheck(1);
        m_serialRdo.SetCheck(0);
        m_debugRdo.SetCheck(0);
    }
    else if (sDialingDir->nType == DCOM_DD_SSH)
    {
        OnBnClickedSshRdo();
        m_sshRdo.SetCheck(1);
        m_telnetRdo.SetCheck(0);
        m_serialRdo.SetCheck(0);
        m_debugRdo.SetCheck(0);
    }
}

void CNewConnDlg::OnBnClickedDialingDirButton()
{
    if (m_dialingDirDlg.m_hWnd == 0)
    {
        if (m_dialingDirDlg.Create(MAKEINTRESOURCE(IDD_DIALING_DIR_DIALOG), this) == 0)
        {
            return;
        }
    }
    m_dialingDirDlg.ShowWindow(SW_SHOW);
}

LRESULT CNewConnDlg::OnDialingDirOK(WPARAM, LPARAM)
{
    CString szUserName;
    m_sDialingDirSelection = m_dialingDirDlg.GetSelected();
    if (m_sDialingDirSelection.nType != DCOM_DD_INVALID)
    {
        m_hostNameCombo.SetWindowText(m_sDialingDirSelection.szDescription);
        PopulateControls(&m_sDialingDirSelection);
    }
    return 0;
}


void CNewConnDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CSizeDialog::OnShowWindow(bShow, nStatus);
    if (bShow == 1)
    {
        DetectComPorts();
    }
}
