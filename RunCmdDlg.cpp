// RunCmd.cpp : implementation file
//

#include "stdafx.h"
#include "DCom.h"
#include "DComDoc.h"
#include "RunCmdDlg.h"

#define DCOM_COMMAND_SECTION   "Commands"
#define DCOM_COMMAND_PATH      "Path"
#define DCOM_COMMAND_NUM_CMDS  "NumCmds"
#define DCOM_COMMAND_COMMAND   "%i - Command"
#define DCOM_COMMAND_OPTIONS   "%i - Options"
#define DCOM_COMMAND_NUM_DIRS  "NumDirs"
#define DCOM_COMMAND_DIRECTORY "%i - Directory"
#define DCOM_COMMAND_WD        "WorkingDir"
#define DCOM_COMMAND_USER      0x01
#define DCOM_COMMAND_STDIN     0x02
#define DCOM_COMMAND_PIPEIN    0x04

// CRunCmdDlg dialog

IMPLEMENT_DYNAMIC(CRunCmdDlg, CSizeDialog)

CRunCmdDlg::CRunCmdDlg(CWnd* pParent /*=NULL*/)
	: CSizeDialog(CRunCmdDlg::IDD, pParent)
{

}

CRunCmdDlg::~CRunCmdDlg()
{
}

void CRunCmdDlg::DoDataExchange(CDataExchange* pDX)
{
    CSizeDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STDIN, m_stdin);
    DDX_Control(pDX, IDC_USER_IN, m_userin);
    DDX_Control(pDX, IDC_COMMAND_COMBO, m_commandCombo);
    DDX_Control(pDX, IDC_WORKING_DIR_COMBO, m_workingDirCombo);
    DDX_Control(pDX, IDC_PIPEIN, m_pipein);
}


BEGIN_MESSAGE_MAP(CRunCmdDlg, CSizeDialog)
    ON_BN_CLICKED(IDOK, &CRunCmdDlg::OnBnClickedOk)
    ON_CBN_SELCHANGE(IDC_COMMAND_COMBO, &CRunCmdDlg::OnCbnSelchangeCommandCombo)
    ON_BN_CLICKED(ID_RUN_BROWSE, &CRunCmdDlg::OnBnClickedRunBrowse)
    ON_WM_GETMINMAXINFO()
    ON_BN_CLICKED(ID_RUN_BROWSE_WD, &CRunCmdDlg::OnBnClickedRunBrowseWd)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CRunCmdDlg)
    EASYSIZE(IDC_WORKING_DIR_COMBO, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
    EASYSIZE(IDC_COMMAND_COMBO, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
    EASYSIZE(IDCANCEL,          ES_KEEPSIZE, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
    EASYSIZE(IDOK,              ES_KEEPSIZE, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
    EASYSIZE(IDC_STDIN,         ES_KEEPSIZE, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
    EASYSIZE(IDC_PIPEIN,        ES_KEEPSIZE, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
    EASYSIZE(IDC_USER_IN,       ES_KEEPSIZE, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
    EASYSIZE(ID_RUN_BROWSE_CMD, ES_KEEPSIZE, ES_BORDER,   ES_BORDER, ES_KEEPSIZE, 0)
    EASYSIZE(ID_RUN_BROWSE_WD,  ES_KEEPSIZE, ES_BORDER,   ES_BORDER, ES_KEEPSIZE, 0)
END_EASYSIZE_MAP


// CRunCmdDlg message handlers

char *CRunCmdDlg::GetSectionName()
{
    return "Run Command";
}

BOOL CRunCmdDlg::OnInitDialog()
{
    CSizeDialog::OnInitDialog();

    m_stdin.SetCheck(BST_UNCHECKED);
    m_pipein.SetCheck(BST_UNCHECKED);
    m_userin.SetCheck(BST_CHECKED);

    GetCommandsFromRegistry();

    return TRUE;
}

void CRunCmdDlg::GetCommandsFromRegistry()
{
    int nIndex;
    CWinApp *pApp;
    char szReg[32];
    CString szCommand;
    CString szDirectory;
    int nOptions;
    int nItem;

    pApp = AfxGetApp();
    if (pApp)
    {
        nIndex = pApp->GetProfileInt(DCOM_COMMAND_SECTION, DCOM_COMMAND_NUM_CMDS, 0);
        while (nIndex-- > 0)
        {
            sprintf(szReg, DCOM_COMMAND_COMMAND, nIndex);
            szCommand = pApp->GetProfileString(DCOM_COMMAND_SECTION, szReg);

            sprintf(szReg, DCOM_COMMAND_OPTIONS, nIndex);
            nOptions = pApp->GetProfileInt(DCOM_COMMAND_SECTION, szReg, 0);


            if (szCommand.GetLength())
            {
                nItem = m_commandCombo.AddString(szCommand);
                m_commandCombo.SetItemData(nItem, nOptions);
            }
        }
        nIndex = pApp->GetProfileInt(DCOM_COMMAND_SECTION, DCOM_COMMAND_NUM_DIRS, 0);
        while (nIndex-- > 0)
        {
            sprintf(szReg, DCOM_COMMAND_DIRECTORY, nIndex);
            szDirectory = pApp->GetProfileString(DCOM_COMMAND_SECTION, szReg);
            if (szDirectory.GetLength())
            {
                m_workingDirCombo.AddString(szDirectory);
            }
        }
    }
}

int CRunCmdDlg::IsInRegistry(char *szKey, char *szCommand, char *szNum)
{
    int nIndex;
    CWinApp *pApp;
    char szReg[32];
    CString szData;

    pApp = AfxGetApp();
    nIndex = pApp->GetProfileInt(DCOM_COMMAND_SECTION, szNum, 0);
    while (nIndex-- > 0)
    {
        sprintf(szReg, szKey, nIndex);
        szData = pApp->GetProfileString(DCOM_COMMAND_SECTION, szReg);

        if (szData.Compare(szCommand) == 0)
        {
            break;
        }
    }
    return nIndex;
}

void CRunCmdDlg::AddCommandToRegistry(CString szCommand, CString szDirectory, bool bStdin, bool bPipein, bool bUser)
{
    int nIndex;
    CWinApp *pApp;
    char szReg[32];
    CString szData;
    int nNewOptions;

    nNewOptions = 0;
    nNewOptions |= (bStdin)  ? DCOM_COMMAND_STDIN    : 0;
    nNewOptions |= (bPipein) ? DCOM_COMMAND_PIPEIN   : 0;
    nNewOptions |= (bUser)   ? DCOM_COMMAND_USER     : 0;
    pApp = AfxGetApp();
    if (pApp)
    {
        if (szCommand.GetLength())
        {
            nIndex = IsInRegistry(DCOM_COMMAND_COMMAND, szCommand.GetBuffer(), DCOM_COMMAND_NUM_CMDS);
            if (nIndex == -1)
            {
                nIndex = pApp->GetProfileInt(DCOM_COMMAND_SECTION, DCOM_COMMAND_NUM_CMDS, 0);
                pApp->WriteProfileInt(DCOM_COMMAND_SECTION, DCOM_COMMAND_NUM_CMDS, nIndex + 1);

                sprintf(szReg, DCOM_COMMAND_COMMAND, nIndex);
                pApp->WriteProfileString(DCOM_COMMAND_SECTION, szReg, szCommand);
            }

            sprintf(szReg, DCOM_COMMAND_OPTIONS, nIndex);
            pApp->WriteProfileInt(DCOM_COMMAND_SECTION, szReg, nNewOptions);

            nIndex = m_commandCombo.FindStringExact(-1, szCommand);
            if (nIndex == CB_ERR)
            {
                nIndex = m_commandCombo.AddString(szCommand);
            }
            if (nIndex >= 0)
            {
                m_commandCombo.SetItemData(nIndex, nNewOptions);
            }
        }
        if (szDirectory.GetLength())
        {
            nIndex = IsInRegistry(DCOM_COMMAND_DIRECTORY, szDirectory.GetBuffer(), DCOM_COMMAND_NUM_DIRS);
            if (nIndex == -1)
            {
                nIndex = pApp->GetProfileInt(DCOM_COMMAND_SECTION, DCOM_COMMAND_NUM_DIRS, 0);
                pApp->WriteProfileInt(DCOM_COMMAND_SECTION, DCOM_COMMAND_NUM_DIRS, nIndex + 1);

                sprintf(szReg, DCOM_COMMAND_DIRECTORY, nIndex);
                pApp->WriteProfileString(DCOM_COMMAND_SECTION, szReg, szDirectory);
                
                if (m_workingDirCombo.FindStringExact(-1, szDirectory) == CB_ERR)
                {
                    m_workingDirCombo.AddString(szDirectory);
                }
            }
        }
    }
}

void CRunCmdDlg::OnBnClickedOk()
{
    CString szCommand;
    CString szDirectory;
    CDComDoc* pDoc = NULL;
    CFrameWnd* pFrame;

    pFrame = ((CMDIFrameWnd*)AfxGetMainWnd())->MDIGetActive();
    if(NULL != pFrame)
    {
        pDoc = (CDComDoc*)pFrame->GetActiveDocument();
        if (pDoc)
        {
            m_commandCombo.GetWindowText(szCommand);
            m_workingDirCombo.GetWindowText(szDirectory);
            if (pDoc->RunCommand(szCommand.GetBuffer(), szDirectory,
                (m_stdin.GetCheck() == BST_CHECKED),
                (m_pipein.GetCheck() == BST_CHECKED),
                (m_userin.GetCheck() == BST_CHECKED)) == TRUE)
            {
                AddCommandToRegistry(szCommand, szDirectory,
                    (m_stdin.GetCheck() == BST_CHECKED),
                    (m_pipein.GetCheck() == BST_CHECKED),
                    (m_userin.GetCheck() == BST_CHECKED));
            }
        }
    }

    OnOK();
}

void CRunCmdDlg::OnCbnSelchangeCommandCombo()
{
    int nOptions;
    nOptions = m_commandCombo.GetItemData(m_commandCombo.GetCurSel());
    m_stdin.SetCheck  ((nOptions & DCOM_COMMAND_STDIN   ) ? BST_CHECKED : BST_UNCHECKED);
    m_pipein.SetCheck ((nOptions & DCOM_COMMAND_PIPEIN  ) ? BST_CHECKED : BST_UNCHECKED);
    m_userin.SetCheck ((nOptions & DCOM_COMMAND_USER    ) ? BST_CHECKED : BST_UNCHECKED);
}

#define DCOM_FILECLIPBOARD_EXTS "Executables (*.exe;*.bat)|*.exe;*.bat|All Files (*.*)|*.*||"

void CRunCmdDlg::OnBnClickedRunBrowse()
{
    CString szFilename;
    CString szInitialPath;
    CFileDialog fileDlg(TRUE, NULL, NULL, 
        OFN_HIDEREADONLY | OFN_ENABLESIZING | OFN_FILEMUSTEXIST, DCOM_FILECLIPBOARD_EXTS, this);
    
    szInitialPath = AfxGetApp()->GetProfileString(DCOM_COMMAND_SECTION, DCOM_COMMAND_PATH, "");
    if (szInitialPath.GetLength())
    {
        fileDlg.m_ofn.lpstrInitialDir = szInitialPath;
    }
    if (fileDlg.DoModal() == IDOK)
    {
        szFilename = fileDlg.GetPathName();
        AfxGetApp()->WriteProfileString(DCOM_COMMAND_SECTION, DCOM_COMMAND_PATH, szFilename);
        m_commandCombo.SetWindowText(szFilename);
    }
}

INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData) 
{
    CString szDir;
    UNREFERENCED_PARAMETER(lp);

    switch(uMsg) 
    {
    case BFFM_INITIALIZED:
        // Attempt to read the current path from the registry 
        szDir = AfxGetApp()->GetProfileString(DCOM_COMMAND_SECTION, DCOM_COMMAND_WD);
        if (szDir.GetLength() > 0)
        {
            // WParam is TRUE since you are passing a path.
            // It would be FALSE if you were passing a pidl.
            SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)szDir.GetBuffer(255));
        }
        break;
    }
    return 0;
}

void CRunCmdDlg::OnBnClickedRunBrowseWd()
{
    BROWSEINFO bi = { 0 };
    CString szProfile;
    CString szPathname;
    LPITEMIDLIST pidl;


    bi.lpszTitle = _T("Pick a Directory:");
    bi.ulFlags = BIF_NEWDIALOGSTYLE | BIF_VALIDATE;
    bi.lpfn = BrowseCallbackProc;
    bi.lParam = 0;
    pidl = SHBrowseForFolder (&bi);
    if (pidl != 0)
    {
        // get the name of the folder
        TCHAR path[MAX_PATH];
        if (SHGetPathFromIDList(pidl, path))
        {
            szPathname.Format(path);
            AfxGetApp()->WriteProfileString(DCOM_COMMAND_SECTION, DCOM_COMMAND_WD, szPathname.GetBuffer(255));
            m_workingDirCombo.SetWindowText(szPathname);
        }

        // free memory used
        IMalloc * imalloc = 0;
        if (SUCCEEDED(SHGetMalloc(&imalloc)))
        {
            imalloc->Free(pidl);
            imalloc->Release();
        }
    }
}

void CRunCmdDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
    if (m_initialRect.Height() > 0)
    {
        lpMMI->ptMaxTrackSize.y = m_initialRect.Height();
    }
    CSizeDialog::OnGetMinMaxInfo(lpMMI);
}


