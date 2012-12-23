// TitleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DCom.h"
#include "TitleDlg.h"


// CTitleDlg dialog

IMPLEMENT_DYNAMIC(CTitleDlg, CDialog)

CTitleDlg::CTitleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTitleDlg::IDD, pParent)
    , m_szTitleText(_T(""))
{

}

CTitleDlg::~CTitleDlg()
{
}

void CTitleDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TITLE_EDIT, m_cTitleEdit);
}


BEGIN_MESSAGE_MAP(CTitleDlg, CDialog)
    ON_BN_CLICKED(IDOK, &CTitleDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CTitleDlg message handlers

void CTitleDlg::OnBnClickedOk()
{
    m_cTitleEdit.GetWindowText(m_szTitleText);
    OnOK();
}

BOOL CTitleDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    if (m_szTitleText.GetLength() > 0)
    {
        m_cTitleEdit.SetWindowText(m_szTitleText);
    }
    return TRUE;
}
