// CharMapDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DCom.h"
#include "CharMapDlg.h"


// CCharMapDlg dialog

IMPLEMENT_DYNAMIC(CCharMapDlg, CSizeDialog)

CCharMapDlg::CCharMapDlg(CWnd* pParent /*=NULL*/)
	: CSizeDialog(CCharMapDlg::IDD, pParent)
{

}

CCharMapDlg::~CCharMapDlg()
{
}

void CCharMapDlg::DoDataExchange(CDataExchange* pDX)
{
    CSizeDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CHARMAP_LIST, m_cCharMapList);
}

char *CCharMapDlg::GetSectionName()
{
    return "CharMap";
}

BEGIN_MESSAGE_MAP(CCharMapDlg, CSizeDialog)
    ON_BN_CLICKED(IDOK, &CCharMapDlg::OnBnClickedOk)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CCharMapDlg)
    EASYSIZE(IDOK,                       ES_KEEPSIZE,   ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
    EASYSIZE(IDCANCEL,                   ES_KEEPSIZE,   ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
    EASYSIZE(IDC_CHARMAP_LIST,           ES_BORDER  ,   ES_BORDER  , ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

// CCharMapDlg message handlers

BOOL CCharMapDlg::OnInitDialog()
{
    CSizeDialog::OnInitDialog();

    char szTmp[32];
    int nIndex;
    m_cCharMapList.InsertColumn(0, "Char", LVCFMT_LEFT, 100);
    m_cCharMapList.InsertColumn(1, "Ascii", LVCFMT_LEFT, 100);
    for (int i = 1; i < 256; i++)
    {
        sprintf(szTmp, "%c        ", i);
        nIndex = m_cCharMapList.InsertItem(i, szTmp);
        sprintf(szTmp, "%i", i);
        m_cCharMapList.SetItemText(nIndex, 1, szTmp);
    }
    return TRUE;
}
char CCharMapDlg::GetSelectedChar()
{
    POSITION pos;
    int nItem;
    int nRet;
    char szTmp[32];

    pos = m_cCharMapList.GetFirstSelectedItemPosition();
    nItem = m_cCharMapList.GetNextSelectedItem(pos);
    m_cCharMapList.GetItemText(nItem, 1, szTmp, sizeof(szTmp));
    sscanf(szTmp, "%i", &nRet);
    return (char)nRet;
}

void CCharMapDlg::OnBnClickedOk()
{
    GetParent()->PostMessage(WM_CHARMAP_OK, GetSelectedChar());
    OnOK();
}
