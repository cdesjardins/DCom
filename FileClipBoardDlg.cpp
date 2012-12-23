// FileClipboardDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DCom.h"
#include "FileClipboardDlg.h"
#include "DComDoc.h"
#include <algorithm>

#define DCOM_FILECLIPBOARD_EXTS       "DCom File Clipboard (*.tfc)|*.tfc|All Files (*.*)|*.*||"
#define DCOM_FILECLIPBOARD_SECTION    "FileClipboard"
#define DCOM_FILECLIPBOARD_PATH       "Path"
#define DCOM_FILECLIPBOARD_ITEM       "Item%i"
#define DCOM_FILECLIPBOARD_ITEM_CNT   "ItemCount"
#define DCOM_FILECLIPBOARD_NEWLINE    "Newline"

// CFileClipboardDlg dialog

IMPLEMENT_DYNAMIC(CFileClipboardDlg, CSizeDialog)

CFileClipboardDlg::CFileClipboardDlg(CWnd* pParent /*=NULL*/)
	: CSizeDialog(CFileClipboardDlg::IDD, pParent)
{
}

CFileClipboardDlg::~CFileClipboardDlg()
{
}

char *CFileClipboardDlg::GetSectionName()
{
    return DCOM_FILECLIPBOARD_SECTION;
}

void CFileClipboardDlg::DoDataExchange(CDataExchange* pDX)
{
    CSizeDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_FILE_CLIPBOARD_LIST, m_cFileClipboardList);
    DDX_Control(pDX, IDC_FILE_CLIPBOARD_NEWLINE, m_btNewLine);
}


BEGIN_MESSAGE_MAP(CFileClipboardDlg, CSizeDialog)
    ON_BN_CLICKED(ID_FILE_CLIPBOARD_OPEN, &CFileClipboardDlg::OnBnClickedFileClipboardOpen)
    ON_BN_CLICKED(ID_FILE_CLIPBOARD_SAVE, &CFileClipboardDlg::OnBnClickedFileClipboardSave)
    ON_BN_CLICKED(ID_FILE_CLIPBOARD_SEND, &CFileClipboardDlg::OnBnClickedFileClipboardSend)
    ON_BN_CLICKED(ID_FILE_CLIPBOARD_REMOVE, &CFileClipboardDlg::OnBnClickedFileClipboardRemove)
    ON_WM_DESTROY()
    ON_NOTIFY(LVN_KEYDOWN, IDC_FILE_CLIPBOARD_LIST, &CFileClipboardDlg::OnLvnKeydownFileClipboardList)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CFileClipboardDlg)
    EASYSIZE(IDC_FILE_CLIPBOARD_LIST,    ES_BORDER,     ES_BORDER,   ES_BORDER, ES_BORDER, 0)
    EASYSIZE(IDC_FILE_CLIPBOARD_NEWLINE, ES_KEEPSIZE,   ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
    EASYSIZE(ID_FILE_CLIPBOARD_SEND,     ES_KEEPSIZE,   ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
    EASYSIZE(ID_FILE_CLIPBOARD_REMOVE,   ES_KEEPSIZE,   ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
    EASYSIZE(IDOK,                       ES_KEEPSIZE,   ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
    EASYSIZE(ID_FILE_CLIPBOARD_SAVE,     ES_KEEPSIZE,   ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
    EASYSIZE(ID_FILE_CLIPBOARD_OPEN,     ES_KEEPSIZE,   ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP


// CFileClipboardDlg message handlers

BOOL CFileClipboardDlg::OnInitDialog()
{
    CSizeDialog::OnInitDialog();
    CRect rcRect;
    CString szValue;
    CString szEntry;
    int nNumItems;
    int nIndex;
    int nCheck;

    m_cFileClipboardList.InsertColumn(0, "", LVCFMT_LEFT, 0x500);
    m_cFileClipboardList.InsertItem(0, "");

    nNumItems = AfxGetApp()->GetProfileInt(DCOM_FILECLIPBOARD_SECTION, DCOM_FILECLIPBOARD_ITEM_CNT, 0);
    nCheck = AfxGetApp()->GetProfileInt(DCOM_FILECLIPBOARD_SECTION, DCOM_FILECLIPBOARD_NEWLINE, BST_CHECKED);
    m_btNewLine.SetCheck(nCheck);
    for (nIndex = 0; nIndex < nNumItems; nIndex++)
    {
        szEntry.Format(DCOM_FILECLIPBOARD_ITEM, nIndex);
        szValue = AfxGetApp()->GetProfileString(DCOM_FILECLIPBOARD_SECTION, szEntry, "");
        if (szValue.GetLength() > 0)
        {
            m_cFileClipboardList.InsertItem(m_cFileClipboardList.GetItemCount() - 1, szValue);
        }
    }

    return TRUE;
}

BOOL CFileClipboardDlg::GetFileClipboardFilename(BOOL bOpen, CString *szFilename)
{
    DWORD dwFlags = OFN_HIDEREADONLY | OFN_ENABLESIZING;
    if (bOpen)
    {
        dwFlags |= OFN_FILEMUSTEXIST;
    }
    CString szInitialPath;
    CFileDialog fileDlg( bOpen, NULL, NULL, 
        dwFlags, DCOM_FILECLIPBOARD_EXTS, this);
    szInitialPath = AfxGetApp()->GetProfileString(DCOM_FILECLIPBOARD_SECTION, DCOM_FILECLIPBOARD_PATH, "");
    if (szInitialPath.GetLength())
    {
        fileDlg.m_ofn.lpstrInitialDir = szInitialPath;
    }
    if (fileDlg.DoModal() == IDOK)
    {
        *szFilename = fileDlg.GetPathName();
        AfxGetApp()->WriteProfileString(DCOM_FILECLIPBOARD_SECTION, DCOM_FILECLIPBOARD_PATH, *szFilename);
        return TRUE;
    }
    return FALSE;
}

void CFileClipboardDlg::ReadFileClipboard(CString szFilename)
{
    CStdioFile fInput;
    UINT  nFileSize;
    char *szFileData;
    char *pEnd, *pStart;

    if (fInput.Open(szFilename, CFile::typeText|CFile::modeRead) == TRUE)
    {
        nFileSize = (UINT)fInput.GetLength();
        szFileData = (char*)malloc(nFileSize + 1);
        if (szFileData)
        {
            nFileSize = fInput.Read(szFileData, nFileSize);
            if (nFileSize > 0)
            {
                szFileData[nFileSize] = 0;
                pStart = szFileData;
                do
                {
                    pEnd = strchr(pStart, '\n');
                    if (pEnd)
                    {
                        *pEnd = 0;
                        pEnd++;
                    }
                    if (strlen(pStart) > 0)
                    {
                        m_cFileClipboardList.InsertItem(m_cFileClipboardList.GetItemCount() - 1, pStart);
                    }
                    pStart = pEnd;
                } while (pStart);
            }
            free(szFileData);
        }
        fInput.Close();
    }
}

void CFileClipboardDlg::OnBnClickedFileClipboardOpen()
{
    CString szFilename;

    if (GetFileClipboardFilename(TRUE, &szFilename) == TRUE)
    {
        ReadFileClipboard(szFilename);
    }
}

void CFileClipboardDlg::OnBnClickedFileClipboardSave()
{
    CString szFilename;
    CStdioFile fOutput;
    int nIndex;
    CString szText;

    if (GetFileClipboardFilename(FALSE, &szFilename) == TRUE)
    {
        if (fOutput.Open(szFilename, CFile::typeText|CFile::modeWrite|CFile::modeCreate) == TRUE)
        {
            for (nIndex = 0; nIndex < m_cFileClipboardList.GetItemCount() - 1; nIndex++)
            {
                szText = m_cFileClipboardList.GetItemText(nIndex, 0);
                fOutput.Write(szText.GetBuffer(), szText.GetLength());
                fOutput.Write("\n", 1);
            }
            fOutput.Close();
        }
    }
}

void CFileClipboardDlg::OnBnClickedFileClipboardSend()
{
    m_cFileClipboardList.SendSelectedText();
}

bool backwardsSort(const int& d1, const int& d2)
{
  return d1 > d2;
}

void CFileClipboardDlg::OnBnClickedFileClipboardRemove()
{
    std::vector<int>::iterator vpos;
    std::vector<int> removers;
    POSITION pos = m_cFileClipboardList.GetFirstSelectedItemPosition();
    CString szText;

    while (pos)
    {
        removers.push_back(m_cFileClipboardList.GetNextSelectedItem(pos));
    }
    std::sort(removers.begin(), removers.end(), backwardsSort);
    for (vpos = removers.begin(); vpos != removers.end(); vpos++)
    {
        m_cFileClipboardList.DeleteItem(*vpos);
    }

    if (m_cFileClipboardList.GetItemCount() == 0)
    {
        m_cFileClipboardList.InsertItem(0, "");
    }
    else
    {
        szText = m_cFileClipboardList.GetItemText(m_cFileClipboardList.GetItemCount() - 1, 0);
        if (szText.GetLength() != 0)
        {
            m_cFileClipboardList.InsertItem(m_cFileClipboardList.GetItemCount(), "");
        }
    }
}

void CFileClipboardDlg::OnDestroy()
{
    CSizeDialog::OnDestroy();
    int nIndex;
    CString szText;
    CString szEntry;
    CWinApp *pApp;

    pApp = AfxGetApp();
    if (pApp)
    {
        pApp->WriteProfileInt(DCOM_FILECLIPBOARD_SECTION, DCOM_FILECLIPBOARD_ITEM_CNT, m_cFileClipboardList.GetItemCount() - 1);

        for (nIndex = 0; nIndex < m_cFileClipboardList.GetItemCount() - 1; nIndex++)
        {
            szText = m_cFileClipboardList.GetItemText(nIndex, 0);
            szEntry.Format(DCOM_FILECLIPBOARD_ITEM, nIndex);
            pApp->WriteProfileString(DCOM_FILECLIPBOARD_SECTION, szEntry, szText);
        }
        pApp->WriteProfileInt(DCOM_FILECLIPBOARD_SECTION, DCOM_FILECLIPBOARD_NEWLINE, m_btNewLine.GetCheck());
    }
}

void CFileClipboardDlg::OnLvnKeydownFileClipboardList(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
    if (pLVKeyDow->wVKey == VK_F2)
    {
        POSITION pos = m_cFileClipboardList.GetFirstSelectedItemPosition();
        if (pos)
        {
            m_cFileClipboardList.EditLabel(m_cFileClipboardList.GetNextSelectedItem(pos));
        }
    }
    *pResult = 0;
}

void CFileClipboardDlg::OnOK()
{
    CSizeDialog::OnOK();
}

BEGIN_MESSAGE_MAP(CMyListCtrl, CListCtrl)
    ON_NOTIFY_REFLECT(LVN_ENDLABELEDIT, &CMyListCtrl::OnLvnEndlabeledit)
    ON_WM_LBUTTONDBLCLK()
    ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, &CMyListCtrl::OnLvnItemchanged)
END_MESSAGE_MAP()

CMyListCtrl::CMyListCtrl()
{
    m_vecSelection.clear();
}

void CMyListCtrl::OnLvnEndlabeledit(NMHDR *pNMHDR, LRESULT *pResult)
{
    NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
    *pResult = 0;
    
    if (pDispInfo->item.mask & LVIF_TEXT)
    {
        if (pDispInfo->item.iItem == (GetItemCount() - 1))
        {
            InsertItem(pDispInfo->item.iItem, pDispInfo->item.pszText);
        }
        else
        {
            if (strlen(pDispInfo->item.pszText) > 0)
            {
                SetItemText(pDispInfo->item.iItem, 0, pDispInfo->item.pszText);
            }
            else
            {
                GetParent()->PostMessage(WM_COMMAND, MAKEWPARAM(ID_FILE_CLIPBOARD_REMOVE, BN_CLICKED));
            }
        }
    }
}

void CMyListCtrl::SendText(int nItem)
{
    CDComView* pView;
    CDComDoc* pDoc = NULL;
    CFrameWnd* pFrame;
    CString szText;
    CFileClipboardDlg *pParent;
    bool bSendNewLine = false;
    pFrame = ((CMDIFrameWnd*)AfxGetMainWnd())->MDIGetActive();
    if(NULL != pFrame)
    {
        pDoc = (CDComDoc*)pFrame->GetActiveDocument();
        if (pDoc)
        {
            pView = pDoc->GetView();
            if (pView)
            {
                pParent = (CFileClipboardDlg*)GetParent();
                if (pParent)
                {
                    bSendNewLine = (pParent->m_btNewLine.GetCheck() == BST_CHECKED) ? true : false;
                }
                szText = GetItemText(nItem, 0);
                if (bSendNewLine)
                {
                    szText += "\r";
                }
                pView->AddUserInput(szText.GetBuffer(), szText.GetLength());
                pView->SetFocus();
            }
        }
    }
}

void CMyListCtrl::SendSelectedText()
{
    std::list<int>::iterator pos;
    for (pos = m_vecSelection.begin(); pos != m_vecSelection.end(); pos++)
    {
        SendText(*pos);
    }
}

void CMyListCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    POSITION pos = GetFirstSelectedItemPosition();
    if (pos)
    {
        SendText(GetNextSelectedItem(pos));
    }
    CListCtrl::OnLButtonDblClk(nFlags, point);
}

void CMyListCtrl::OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    *pResult = 0;

    /* If a new imt addr is selected that was not selected before */
    if (((pNMLV->uNewState & LVIS_SELECTED) == LVIS_SELECTED) &&
        ((pNMLV->uOldState & LVIS_SELECTED) == 0))
    {
        /* Then add it to the imt address vector */
        m_vecSelection.push_back(pNMLV->iItem);
    }
    /* If an imt addr is un-selected */
    else if (((pNMLV->uOldState & LVIS_SELECTED) == LVIS_SELECTED) &&
             ((pNMLV->uNewState & LVIS_SELECTED) == 0))
    {
        /* Then remove it from the list */
        m_vecSelection.remove(pNMLV->iItem);
    }
}
