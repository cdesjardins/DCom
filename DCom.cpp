// DCom.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "DCom.h"
#include "MainFrm.h"

#include "ChildFrm.h"
#include "DComDoc.h"
#include "DComView.h"
#include "afxwin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define DCOM_FILES_ACTION             "DCom"
#define DCOM_FILES_COMMAND            "DCom\\shell\\open\\command"
#define DCOM_FILES_DESCRIPTION        "DCom Terminal Emulator"

// CDComApp

BEGIN_MESSAGE_MAP(CDComApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CDComApp::OnAppAbout)
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()


void CCustomCommandLineInfo::ParseParam(const char* pszParam, BOOL bFlag, BOOL bLast)
{
    char *pDot;
    pDot = strrchr((char*)pszParam, '.');
    if (pDot)
    {
        if (_stricmp(pDot, DCOM_FILES_DIALING_DIR) == 0)
        {
            m_nCustomShellCommand = eFileOpenDialingDir;
            m_strDialingDirFileName.Format("%s", pszParam);
        }
        else if (_stricmp(pDot, DCOM_FILES_TERMINAL_SESSION) == 0)
        {
            m_nCustomShellCommand = eFileOpenTerminalSession;
            m_strTerminalSessionFileName.Format("%s", pszParam);
        }
    }
}

CDComApp::CDComApp()
{
}

CDComApp theApp;

BOOL CDComApp::InitInstance()
{
    AfxInitRichEdit2();
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();

    SetRegistryKey(_T("Delec"));
	LoadStdProfileSettings(0);
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_DComTYPE,
		RUNTIME_CLASS(CDComDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CDComView));
	if (!pDocTemplate)
    {
		return FALSE;
    }
	AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (pMainFrame)
	{
        if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
        {
    		delete pMainFrame;
	    	return FALSE;
        }
	}
    else
    {
        return FALSE;
    }
	m_pMainWnd = pMainFrame;
	// call DragAcceptFiles only if there's a suffix
	//  In an MDI app, this should occur immediately after setting m_pMainWnd

    RegCreateAction();
    RegAssociateFiles(DCOM_FILES_DIALING_DIR);
    RegAssociateFiles(DCOM_FILES_TERMINAL_SESSION);

	// Parse command line for standard shell commands, DDE, file open
	CCustomCommandLineInfo cmdInfo;
    cmdInfo.m_nShellCommand = CCustomCommandLineInfo::FileNothing;
    cmdInfo.m_nCustomShellCommand = CCustomCommandLineInfo::eFileOpenNothing;
	ParseCommandLine(cmdInfo);


	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
    {
		return FALSE;
    }
    if (!ProcessCustomShellCommands(cmdInfo))
    {
        return FALSE;
    }
	// The main window has been initialized, so show and update it
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}

BOOL CDComApp::ProcessCustomShellCommands(CCustomCommandLineInfo& rCmdInfo)
{
	BOOL bResult = TRUE;
	switch (rCmdInfo.m_nCustomShellCommand)
	{
	case CCustomCommandLineInfo::eFileOpenDialingDir:
        WriteProfileString(DCOM_DIALINGDIR_SECTION, DCOM_DIALINGDIR_PATH, rCmdInfo.m_strDialingDirFileName);
        break;
    case CCustomCommandLineInfo::eFileOpenTerminalSession:
        ((CMainFrame*)m_pMainWnd)->OpenTerminals(rCmdInfo.m_strTerminalSessionFileName);
        break;
    }
    return TRUE;
}



BOOL CDComApp::RegKeyExists(LPCTSTR lpSubKey)
{
    LONG nRet;
    BOOL bRet = FALSE;
    HKEY hkResult;

    nRet = RegOpenKeyEx(HKEY_CLASSES_ROOT, lpSubKey, 0, KEY_READ, &hkResult);
    if (nRet == ERROR_SUCCESS)
    {
        RegCloseKey(hkResult);
        bRet = TRUE;
    }
    return bRet;
}

void CDComApp::RegAssociateFiles(LPCTSTR lpSubKey)
{
    LONG nRet;
    HKEY hkResult;

    if (RegKeyExists(lpSubKey) == FALSE)
    {
        nRet = RegCreateKeyEx(HKEY_CLASSES_ROOT, lpSubKey, 0, NULL, 
            REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkResult, NULL);
        if (nRet == ERROR_SUCCESS)
        {
            nRet = RegSetValueEx(hkResult, NULL, 0, REG_SZ, (BYTE*)DCOM_FILES_ACTION, strlen(DCOM_FILES_ACTION));
            RegCloseKey(hkResult);
        }
    }
}

void CDComApp::RegCreateAction()
{
    LONG nRet;
    HKEY hkResult;
    char szModuleName[_MAX_PATH];
    CString szCommand;

    GetModuleFileName(NULL, szModuleName, sizeof(szModuleName));
    
    szCommand.Format("\"%s\" \"%%1\"", szModuleName);
    nRet = RegCreateKeyEx(HKEY_CLASSES_ROOT, DCOM_FILES_ACTION, 0, NULL, 
        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkResult, NULL);
    if (nRet == ERROR_SUCCESS)
    {
        nRet = RegSetValueEx(hkResult, NULL, 0, REG_SZ, (BYTE*)DCOM_FILES_DESCRIPTION, strlen(DCOM_FILES_DESCRIPTION));
        RegCloseKey(hkResult);
    }
    nRet = RegCreateKeyEx(HKEY_CLASSES_ROOT, DCOM_FILES_COMMAND, 0, NULL, 
        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkResult, NULL);
    if (nRet == ERROR_SUCCESS)
    {
        nRet = RegSetValueEx(hkResult, NULL, 0, REG_SZ, (BYTE*)szCommand.GetBuffer(), szCommand.GetLength());
        RegCloseKey(hkResult);
    }
}

void ShowHelp(int nHelpId)
{
    HWND hRet;
    char szModuleName[_MAX_PATH];
    char *pDot;

    hRet = ::HtmlHelp((HWND)NULL,
        "DCom.chm", HH_HELP_CONTEXT, nHelpId);
    if (hRet == NULL)
    {
        GetModuleFileName(NULL, szModuleName, sizeof(szModuleName));
        pDot = strrchr(szModuleName, '.');
        if (pDot)
        {
            *pDot = 0;
            strcat(szModuleName, ".chm");
        }
        hRet = ::HtmlHelp((HWND)NULL,
            szModuleName, HH_HELP_CONTEXT, nHelpId);
    }
}
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();
    virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
    CStatic m_versionStatic;
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_VERSION_STATIC, m_versionStatic);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CDComApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CDComApp message handlers

#define DCOM_MAJOR 1
#define DCOM_MINOR 9

BOOL CAboutDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    char szVersion[128];
    sprintf(szVersion, "DCom, Version %i.%i\nCopyright (c) 2010\nChris Desjardins - Delec\nBuild: %s %s", DCOM_MAJOR, DCOM_MINOR, __DATE__, __TIME__);
    m_versionStatic.SetWindowText(szVersion);
    return TRUE;
}
