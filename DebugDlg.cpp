// DebugDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DCom.h"
#include "DebugDlg.h"


// CDebugDlg dialog

IMPLEMENT_DYNAMIC(CDebugDlg, CSizeDialog)

BEGIN_EASYSIZE_MAP(CDebugDlg)
    EASYSIZE(IDC_DEBUG_EDIT,    ES_BORDER,     ES_BORDER,   ES_BORDER, ES_BORDER, 0)
    EASYSIZE(ID_DEBUG_PAUSE,    ES_KEEPSIZE,   ES_KEEPSIZE, ES_BORDER, ES_BORDER ,0)
    EASYSIZE(ID_DEBUG_CLEAR,    ES_KEEPSIZE,   ES_KEEPSIZE, ES_BORDER, ES_BORDER ,0)
    EASYSIZE(IDOK,              ES_KEEPSIZE,   ES_KEEPSIZE, ES_BORDER, ES_BORDER ,0)
END_EASYSIZE_MAP

CDebugDlg::CDebugDlg(CWnd* pParent /*=NULL*/)
	: CSizeDialog(CDebugDlg::IDD, pParent)
{
    m_bPaused = false;
    m_nChars = 0;
}

CDebugDlg::~CDebugDlg()
{
}

void CDebugDlg::DoDataExchange(CDataExchange* pDX)
{
    CSizeDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_DEBUG_EDIT, m_debugEdit);
    DDX_Control(pDX, ID_DEBUG_PAUSE, m_pauseButton);
}


BEGIN_MESSAGE_MAP(CDebugDlg, CSizeDialog)
    ON_BN_CLICKED(ID_DEBUG_PAUSE, &CDebugDlg::OnBnClickedDebugPause)
    ON_BN_CLICKED(ID_DEBUG_CLEAR, &CDebugDlg::OnBnClickedDebugClear)
END_MESSAGE_MAP()


// CDebugDlg message handlers
void CDebugDlg::DebugOutput(const char *szFormat, ...)
{
    int nLen;
    char *szData;
    va_list vl;
    if (m_bPaused == false)
    {
        va_start(vl, szFormat);
        nLen = _vscprintf(szFormat, vl);
        szData = (char*)malloc(nLen + 1);
        if (szData)
        {
            vsprintf(szData, szFormat, vl);
            m_debugEdit.SetSel(m_nChars, -1);
            m_debugEdit.ReplaceSel(szData);
            m_nChars += nLen;
            free(szData);
        }
        va_end(vl);
    }
}

void CDebugDlg::OnBnClickedDebugPause()
{
    m_bPaused = ! m_bPaused;
    if (m_bPaused)
    {
        m_pauseButton.SetWindowText("Unpause");
    }
    else
    {
        m_pauseButton.SetWindowText("Pause");
    }
}

BOOL CDebugDlg::OnInitDialog()
{
    CSizeDialog::OnInitDialog();

    m_font.CreateFont(12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, _T("Lucida Console"));
    m_debugEdit.SetFont(&m_font);
    return TRUE;
}

void CDebugDlg::OnBnClickedDebugClear()
{
    m_debugEdit.SetWindowText("");
}
