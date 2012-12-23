// ChildFrm.cpp : implementation of the CChildFrame class
//
#include "stdafx.h"
#include "DCom.h"
#include "DComDoc.h"

#include "ChildFrm.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
    ON_WM_CREATE()
    ON_UPDATE_COMMAND_UI(ID_INDICATOR_TX, OnUpdateTx)
    ON_UPDATE_COMMAND_UI(ID_INDICATOR_RX, OnUpdateRx)
END_MESSAGE_MAP()


static UINT indicators[] =
{
    ID_SEPARATOR,           // status line indicator
    ID_INDICATOR_TX,
    ID_INDICATOR_RX,
};

// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
	// TODO: add member initialization code here
}

CChildFrame::~CChildFrame()
{
}


BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    SIZE fontSize;
    CMainFrame *pMainFrm = (CMainFrame *)AfxGetMainWnd();
    if (pMainFrm)
    {
        fontSize = pMainFrm->GetLineDimentions("a", 1);
        cs.cx = 84 * fontSize.cx;
        cs.cy = 30 * fontSize.cy;
    }
    if (!CMDIChildWnd::PreCreateWindow(cs))
    {
		return FALSE;
    }

	return TRUE;
}


// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG


// CChildFrame message handlers

BOOL CChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), TRUE);
    return CMDIChildWnd::OnCreateClient(lpcs, pContext);
}

int CChildFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

    if (!m_wndStatusBar.Create(this, WS_CHILD | WS_VISIBLE | CBRS_BOTTOM, ID_MY_STATUS_BAR) ||
        !m_wndStatusBar.SetIndicators(indicators,
          sizeof(indicators)/sizeof(UINT)))
    {
        TRACE0("Failed to create status bar\n");
        return -1;      // fail to create
    }

    return 0;
}

void CChildFrame::OnUpdateRx(CCmdUI *pCmdUI)
{
    CDComDoc* pDoc = NULL;
    char szData[32];
    pDoc = (CDComDoc*)GetActiveDocument();
    if (pDoc)
    {
        sprintf(szData, "Rx %i", pDoc->m_pTgtIntf->TgtGetBytesRx());
    }
    else
    {
        sprintf(szData, "Rx ???");
    }
    m_wndStatusBar.SetPaneText(
        m_wndStatusBar.CommandToIndex(ID_INDICATOR_RX),
        LPCSTR(szData));
    pCmdUI->Enable();
}

void CChildFrame::OnUpdateTx(CCmdUI *pCmdUI)
{
    CDComDoc* pDoc = NULL;
    char szData[32];
    pDoc = (CDComDoc*)GetActiveDocument();
    if (pDoc)
    {
        sprintf(szData, "Tx %i", pDoc->m_pTgtIntf->TgtGetBytesTx());
    }
    else
    {
        sprintf(szData, "Tx ???");
    }
    m_wndStatusBar.SetPaneText(
        m_wndStatusBar.CommandToIndex(ID_INDICATOR_TX),
        LPCSTR(szData));
    pCmdUI->Enable();
}

void CChildFrame::UpdateStatusPane(CString szStatus)
{
    m_wndStatusBar.SetPaneText(m_wndStatusBar.CommandToIndex(ID_SEPARATOR), szStatus);
}
