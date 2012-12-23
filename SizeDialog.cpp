// SizeDialog.cpp : implementation file
//

#include "stdafx.h"
#include "DCom.h"
#include "SizeDialog.h"

#define SIZE_DLG_WINDOW_PLACEMENT "WindowPlacement"
// CSizeDialog dialog

IMPLEMENT_DYNAMIC(CSizeDialog, CDialog)

CSizeDialog::CSizeDialog(UINT nIDTemplate, CWnd* pParent /*=NULL*/)
	: CDialog(nIDTemplate, pParent)
{

}

CSizeDialog::~CSizeDialog()
{
}

void CSizeDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSizeDialog, CDialog)
    ON_WM_SIZING()
    ON_WM_SIZE()
    ON_WM_HELPINFO()
END_MESSAGE_MAP()


// CSizeDialog message handlers

void CSizeDialog::OnSizing(UINT fwSide, LPRECT pRect)
{
    CDialog::OnSizing(fwSide, pRect);

    EASYSIZE_MINSIZE(m_initialRect.Width(), m_initialRect.Height(), fwSide, pRect);
}

void CSizeDialog::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    UPDATE_EASYSIZE;
}

void CSizeDialog::OnOK()
{
    WINDOWPLACEMENT plWnd;
    GetWindowPlacement(&plWnd);
    AfxGetApp()->WriteProfileBinary(SIZE_DLG_WINDOW_PLACEMENT, GetSectionName(), (LPBYTE)&plWnd, sizeof(plWnd));
    CDialog::OnOK();
}

BOOL CSizeDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    GetWindowRect(m_initialRect);

    INIT_EASYSIZE;

    WINDOWPLACEMENT *plWnd;
    UINT nSize;
    if (AfxGetApp()->GetProfileBinary(SIZE_DLG_WINDOW_PLACEMENT, GetSectionName(), (LPBYTE*)&plWnd, &nSize) == TRUE)
    {
        plWnd->showCmd = SW_HIDE;
        SetWindowPlacement(plWnd);
        delete plWnd;
    }
    SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), TRUE);
    SetWindowText(GetSectionName());

    return TRUE;
}

BOOL CSizeDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{

    if (pHelpInfo->iContextType == HELPINFO_WINDOW) 
    {
        //ShowHelp(GetHelpId());
    }
    return TRUE;
}
