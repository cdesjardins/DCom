// DialingDirDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DCom.h"
#include "NewConnDlg.h"
#include "DialingDirDlg.h"


// CDialingDirDlg dialog

IMPLEMENT_DYNAMIC(CDialingDirDlg, CSizeDialog)

CDialingDirDlg::CDialingDirDlg(CWnd* pParent /*=NULL*/)
	: CSizeDialog(CDialingDirDlg::IDD, pParent)
{

}

CDialingDirDlg::~CDialingDirDlg()
{
}

void CDialingDirDlg::DoDataExchange(CDataExchange* pDX)
{
    CSizeDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_DIALING_DIR_TREE, m_dialingDirTree);
    DDX_Control(pDX, IDCANCEL, m_cancelButton);
    DDX_Control(pDX, IDOK, m_okButton);
    DDX_Control(pDX, ID_OPEN_DIALING_DIR, m_openDialingdirButton);
}


BEGIN_MESSAGE_MAP(CDialingDirDlg, CSizeDialog)
    ON_BN_CLICKED(IDOK, &CDialingDirDlg::OnBnClickedOk)
    ON_BN_CLICKED(ID_OPEN_DIALING_DIR, &CDialingDirDlg::OnBnClickedOpenDialingDir)
    ON_NOTIFY(NM_DBLCLK, IDC_DIALING_DIR_TREE, &CDialingDirDlg::OnNMDblclkDialingDirTree)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CDialingDirDlg)
    EASYSIZE(IDC_DIALING_DIR_TREE,  ES_BORDER,   ES_BORDER  , ES_BORDER, ES_BORDER ,0)
    EASYSIZE(IDCANCEL,              ES_KEEPSIZE, ES_KEEPSIZE, ES_BORDER, ES_BORDER ,0)
    EASYSIZE(IDOK,                  ES_KEEPSIZE, ES_KEEPSIZE, ES_BORDER, ES_BORDER ,0)
    EASYSIZE(ID_OPEN_DIALING_DIR,   ES_KEEPSIZE, ES_KEEPSIZE, ES_BORDER, ES_BORDER ,0)
END_EASYSIZE_MAP

// CDialingDirDlg message handlers

int CDialingDirDlg::GetCurrentLevel()
{
    int nCurrentLevel = 0;
    while (m_szInput[nCurrentLevel] == '\t')
    {
        nCurrentLevel++;
    }
    return nCurrentLevel;
}

void Trim(char *szText)
{
    int nLen = strlen(szText) - 1;
    while ((nLen > 0) && (isspace(szText[nLen])))
    {
        szText[nLen--] = 0;
    }
}

void CDialingDirDlg::RecurseDialingDir(CStdioFile *fDialingDir, int nLevel, HTREEITEM hParent)
{
    SDialingDir sDialingDir;
    HTREEITEM hTreeItem = TVI_ROOT;
    char szDescription[128];
    char *pTok;
    int nCurrentLevel;
    do
    {
        memset(&sDialingDir, 0, sizeof(SDialingDir));
        nCurrentLevel = GetCurrentLevel();
        if (nCurrentLevel > nLevel)
        {
            RecurseDialingDir(fDialingDir, nCurrentLevel, hTreeItem);
            if (strlen(m_szInput) == 0)
            {
                return;
            }
            nCurrentLevel = GetCurrentLevel();
        }
        if (nCurrentLevel < nLevel)
        {
            return;
        }
        strcpy(sDialingDir.szDescription, m_szInput + nCurrentLevel);
        pTok = strtok(m_szInput, " ");
        if (_strnicmp(pTok + nCurrentLevel, DCOM_TELNET_ID, sizeof(DCOM_TELNET_ID)) == 0)
        {
            sDialingDir.nType = DCOM_DD_TELNET;
        }
        else if (_strnicmp(pTok + nCurrentLevel, DCOM_SSH_ID, sizeof(DCOM_SSH_ID)) == 0)
        {
            sDialingDir.nType = DCOM_DD_SSH;
        }
        pTok = strtok(NULL, " ");
        if (pTok)
        {
            strcpy(sDialingDir.szAddress, pTok);
        }
        pTok = strtok(NULL, " ");
        if (pTok)
        {
            sDialingDir.nPort = atoi(pTok);
        }
        pTok = strtok(NULL, "\n");
        if (pTok)
        {
            strcpy(sDialingDir.szDescription, pTok);
        }
        Trim(sDialingDir.szDescription);
        m_sDialingDir.push_back(sDialingDir);
        if (sDialingDir.nPort > 0)
        {
            sprintf(szDescription, "%s - %s %i", sDialingDir.szDescription, sDialingDir.szAddress, sDialingDir.nPort);
        }
        else
        {
            sprintf(szDescription, "%s", sDialingDir.szDescription);
        }
        hTreeItem = m_dialingDirTree.InsertItem(szDescription, hParent);
        m_dialingDirTree.SetItemData(hTreeItem, m_sDialingDir.size() - 1);
        memset(m_szInput, 0, sizeof(m_szInput));
    } while (fDialingDir->ReadString(m_szInput, sizeof(m_szInput)));
}

void CDialingDirDlg::SetupIcons(HTREEITEM hTreeItem)
{
    int nIndex;

    if (hTreeItem)
    {
        if (m_dialingDirTree.ItemHasChildren(hTreeItem))
        {
            m_dialingDirTree.SetItemImage(hTreeItem, m_dotBmIndex, m_dotBmIndex);
            SetupIcons(m_dialingDirTree.GetNextItem(hTreeItem, TVGN_CHILD));
            SetupIcons(m_dialingDirTree.GetNextItem(hTreeItem, TVGN_NEXT));
        }
        else
        {
            do
            {
                nIndex = m_dialingDirTree.GetItemData(hTreeItem);
                if (m_sDialingDir[nIndex].nType == DCOM_DD_TELNET)
                {
                    m_dialingDirTree.SetItemImage(hTreeItem, m_telnetBmIndex, m_telnetBmIndex);
                }
                else
                {
                    m_dialingDirTree.SetItemImage(hTreeItem, m_sshBmIndex, m_sshBmIndex);
                }
                hTreeItem = m_dialingDirTree.GetNextItem(hTreeItem, TVGN_NEXT);
            } while (hTreeItem);
        }
    }
}

void CDialingDirDlg::ParseDialingDir()
{
    CStdioFile fDialingDir;
    HTREEITEM hTreeItem = TVI_ROOT;
    int nLevel = 0;
    m_sDialingDir.clear();
    m_dialingDirTree.DeleteAllItems();
    SetWindowText(m_szDialingDirFilename);
    if (fDialingDir.Open(m_szDialingDirFilename, CFile::modeRead))
    {
        fDialingDir.ReadString(m_szInput, sizeof(m_szInput));
        RecurseDialingDir(&fDialingDir, nLevel, hTreeItem);
    }

    SetupIcons(m_dialingDirTree.GetRootItem());
}

char *CDialingDirDlg::GetSectionName()
{
    return DCOM_DIALINGDIR_SECTION;
}

BOOL CDialingDirDlg::OnInitDialog()
{
    CSizeDialog::OnInitDialog();

    m_szDialingDirFilename = AfxGetApp()->GetProfileString(DCOM_DIALINGDIR_SECTION, DCOM_DIALINGDIR_PATH, "directory.tdd");

    m_imageList.Create(16, 16, ILC_COLOR8, 0, 4);

    CBitmap bm;

    bm.LoadBitmap(IDB_DOT);
    m_dotBmIndex = m_imageList.Add(&bm, RGB(0, 0xff, 0xff));
    bm.DeleteObject();

    bm.LoadBitmap(IDB_SSH);
    m_sshBmIndex = m_imageList.Add(&bm, RGB(0, 0xff, 0xff));
    bm.DeleteObject();

    bm.LoadBitmap(IDB_TELNET);
    m_telnetBmIndex = m_imageList.Add(&bm, RGB(0, 0xff, 0xff));
    bm.DeleteObject();

    m_dialingDirTree.SetImageList(&m_imageList, TVSIL_NORMAL);
    ParseDialingDir();

    return TRUE;
}

SDialingDir CDialingDirDlg::GetSelected()
{
    return m_sSelectedDialingDir;
}

void CDialingDirDlg::OnBnClickedOk()
{
    HTREEITEM hTreeItem;
    int nIndex;

    hTreeItem = m_dialingDirTree.GetSelectedItem();
    if ((hTreeItem) && (m_dialingDirTree.ItemHasChildren(hTreeItem) == FALSE))
    {
        nIndex = m_dialingDirTree.GetItemData(hTreeItem);
        memcpy(&m_sSelectedDialingDir, &m_sDialingDir[nIndex], sizeof(SDialingDir));
    }
    else
    {
        memset(&m_sSelectedDialingDir, 0, sizeof(SDialingDir));
        m_sSelectedDialingDir.nType = DCOM_DD_INVALID;
    }
    GetParent()->PostMessage(WM_DIALING_DIR_OK);

    OnOK();
}

void CDialingDirDlg::OnBnClickedOpenDialingDir()
{
    CString szInitialPath;
    CFileDialog fileDlg(TRUE, NULL, NULL, 
        OFN_HIDEREADONLY | OFN_ENABLESIZING | OFN_FILEMUSTEXIST, 
        DCOM_DIALINGDIR_EXTS, this);
    szInitialPath = AfxGetApp()->GetProfileString(DCOM_DIALINGDIR_SECTION, DCOM_DIALINGDIR_PATH, "");
    if (szInitialPath.GetLength())
    {
        fileDlg.m_ofn.lpstrInitialDir = szInitialPath;
    }
    if (fileDlg.DoModal() == IDOK)
    {
        m_szDialingDirFilename = fileDlg.GetPathName();
        AfxGetApp()->WriteProfileString(DCOM_DIALINGDIR_SECTION, DCOM_DIALINGDIR_PATH, m_szDialingDirFilename);

        ParseDialingDir();
    }
}

void CDialingDirDlg::OnNMDblclkDialingDirTree(NMHDR *pNMHDR, LRESULT *pResult)
{
    PostMessage(WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED));
    *pResult = 0;
}
