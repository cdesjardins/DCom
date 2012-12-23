// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "DCom.h"
#include "DComDoc.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DCOM_TERMINALS_SECTION "Terminals"
#define DCOM_TERMINALS_PATH    "Path"

std::string g_debug_string;
CRITICAL_SECTION g_debug_cs;

// CMainFrame
const UINT CMainFrame::m_FindDialogMessage = RegisterWindowMessage(FINDMSGSTRING);

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	ON_WM_CREATE()
    ON_COMMAND(ID_FILE_MYOPEN, &CMainFrame::OnFileMyopen)
    ON_COMMAND(ID_FILE_RUNCOMMAND, &CMainFrame::OnFileRuncommand)
    ON_COMMAND(ID_CLEAR_SCREEN, &CMainFrame::OnClearScreen)
    ON_COMMAND(ID_OPEN_FILE_CLIPBOARD, &CMainFrame::OnOpenFileClipboard)
    ON_UPDATE_COMMAND_UI(ID_FILE_RUNCOMMAND, &CMainFrame::OnUpdateFileRuncommand)
    ON_UPDATE_COMMAND_UI(ID_OPEN_FILE_CLIPBOARD, &CMainFrame::OnUpdateOpenFileClipboard)
    ON_COMMAND(ID_FILE_CAPTURE, &CMainFrame::OnFileCapture)
    ON_UPDATE_COMMAND_UI(ID_FILE_CAPTURE, &CMainFrame::OnUpdateFileCapture)
    ON_COMMAND(ID_VIEW_OPTIONS, &CMainFrame::OnViewOptions)
    ON_WM_DESTROY()
    ON_COMMAND(ID_EDIT_FIND, &CMainFrame::OnEditFind)
    ON_COMMAND(ID_EDIT_REPEAT, &CMainFrame::OnEditRepeat)
    ON_UPDATE_COMMAND_UI(ID_EDIT_FIND, &CMainFrame::OnUpdateEditFind)
    ON_UPDATE_COMMAND_UI(ID_EDIT_REPEAT, &CMainFrame::OnUpdateEditRepeat)
    ON_REGISTERED_MESSAGE(m_FindDialogMessage, OnFindDialogMessage)
    ON_COMMAND(ID_EDIT_REPEAT_UP, &CMainFrame::OnEditRepeatUp)
    ON_UPDATE_COMMAND_UI(ID_EDIT_REPEAT_UP, &CMainFrame::OnUpdateEditRepeatUp)
    ON_COMMAND(ID_FILE_SAVE_TERMINALS, &CMainFrame::OnFileSaveTerminals)
    ON_COMMAND(ID_FILE_OPEN_TERMINALS, &CMainFrame::OnFileOpenTerminals)
    ON_COMMAND(ID_HELP_HELP, &CMainFrame::OnHelpHelp)
    ON_COMMAND(ID_VIEW_DEBUG, &CMainFrame::OnViewDebug)
    ON_WM_TIMER()
END_MESSAGE_MAP()


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
    struct WSAData wsaData;
    m_bFindMatchCase = FALSE;
    m_bFindSearchDown = TRUE;
    InitializeCriticalSectionAndSpinCount(&g_debug_cs, 0x1000);
    WSAStartup(MAKEWORD(2, 0), &wsaData);
}

CMainFrame::~CMainFrame()
{
    DeleteCriticalSection(&g_debug_cs);
    WSACleanup();
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
    {
		return -1;
    }
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

    if (m_cfgDlg.Create(MAKEINTRESOURCE(IDD_CONFIG_DIALOG), this) == FALSE)
    {
        return -1;
    }
    if (m_debugDlg.Create(MAKEINTRESOURCE(IDD_DEBUG_DIALOG), this) == FALSE)
    {
        return -1;
    }
    m_nTimerID = SetTimer(15, 5, NULL);

	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers



void CMainFrame::OnFileMyopen()
{
    if (m_newConnDlg.m_hWnd == 0)
    {
        if (m_newConnDlg.Create(MAKEINTRESOURCE(IDD_NEW_CONN_DIALOG), this) == 0)
        {
            return;
        }
    }
    m_newConnDlg.ShowWindow(SW_SHOW);
}

void CMainFrame::OnFileRuncommand()
{
    CDComDoc* pDoc = NULL;
    CFrameWnd* pFrame;

    pFrame = ((CMDIFrameWnd*)AfxGetMainWnd())->MDIGetActive();
    if (pFrame)
    {
        pDoc = (CDComDoc*)pFrame->GetActiveDocument();
        if (pDoc)
        {
            if (pDoc->IsRunning() == TRUE)
            {
                if (AfxMessageBox("Stop current program?", MB_YESNO) == IDYES)
                {
                    pDoc->StopRunning();
                }
                return;
            }
        }
    }
    if (m_runCmdDlg.m_hWnd == 0)
    {
        if (m_runCmdDlg.Create(MAKEINTRESOURCE(IDD_COMMAND_DIALOG), this) == 0)
        {
            return;
        }
    }
    m_runCmdDlg.ShowWindow(SW_SHOW);
}

void CMainFrame::OnClearScreen()
{
    CDComDoc* pDoc = NULL;
    CFrameWnd* pFrame;

    pFrame = ((CMDIFrameWnd*)AfxGetMainWnd())->MDIGetActive();
    if(NULL != pFrame)
    {
        pDoc = (CDComDoc*)pFrame->GetActiveDocument();
        if (pDoc)
        {
            pDoc->ClearScreen();
        }
    }
}

void CMainFrame::OnFileCapture()
{
    CDComDoc* pDoc = NULL;
    CFrameWnd* pFrame;

    pFrame = ((CMDIFrameWnd*)AfxGetMainWnd())->MDIGetActive();
    if(NULL != pFrame)
    {
        pDoc = (CDComDoc*)pFrame->GetActiveDocument();
        if (pDoc)
        {
            pDoc->OnFileCapture(this);
        }
    }
}

void CMainFrame::OnOpenFileClipboard()
{
    if (m_fileClipboardDlg.m_hWnd == 0)
    {
        if (m_fileClipboardDlg.Create(MAKEINTRESOURCE(IDD_FILE_CLIPBOARD_DIALOG), this) == 0)
        {
            return;
        }
    }
    m_fileClipboardDlg.ShowWindow(SW_SHOW);
}

void CMainFrame::OnUpdateFileRuncommand(CCmdUI *pCmdUI)
{
    if (MDIGetActive())
    {
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }
}

void CMainFrame::OnUpdateOpenFileClipboard(CCmdUI *pCmdUI)
{
    if (MDIGetActive())
    {
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }
}

void CMainFrame::OnUpdateFileCapture(CCmdUI *pCmdUI)
{
    if (MDIGetActive())
    {
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }
}

void CMainFrame::InvalidateAll()
{
    POSITION appPos, templatePos;
    CDocTemplate *pTemplate;
    CDComDoc *pDoc;
    CDComView *pView;

    appPos = AfxGetApp()->GetFirstDocTemplatePosition();

    while (appPos)
    {
        pTemplate = AfxGetApp()->GetNextDocTemplate(appPos);
        if (pTemplate)
        {
            templatePos = pTemplate->GetFirstDocPosition();
            while (templatePos)
            {
                pDoc = (CDComDoc*)pTemplate->GetNextDoc(templatePos);
                if (pDoc)
                {
                    pView = pDoc->GetView();
                    if (pView)
                    {
                        pView->PostMessage(WM_DOC_UPDATE, 0);
                        pView->Invalidate();
                    }
                }
            }
        }
    }
}

void CMainFrame::SetConfig(SDComCfg *sDComCfg)
{
    POSITION appPos, templatePos;
    CDocTemplate *pTemplate;
    CDComDoc *pDoc;

    appPos = AfxGetApp()->GetFirstDocTemplatePosition();

    while (appPos)
    {
        pTemplate = AfxGetApp()->GetNextDocTemplate(appPos);
        if (pTemplate)
        {
            templatePos = pTemplate->GetFirstDocPosition();
            while (templatePos)
            {
                pDoc = (CDComDoc*)pTemplate->GetNextDoc(templatePos);
                if (pDoc)
                {
                    pDoc->SetConfig(sDComCfg);
                }
            }
        }
    }
}

SIZE CMainFrame::GetLineDimentions(char *pLine, int nLen, CDC *pDC)
{
    SIZE size;
    SIZE sizeRet;
    CFont font;
    SDComCfg *pDComCfg;
    bool bRelease = false;
    if (pDC == NULL)
    {
        pDC = GetDC();
        bRelease = true;
    }
    pDC->SetMapMode(MM_TEXT);
    pDComCfg = GetConfig();
    font.CreateFontIndirect(&pDComCfg->m_logfont);
    pDC->SelectObject(font);
    pDC->GetOutputTextMetrics(&m_stFontMetrics);
    GetTextExtentPoint32(pDC->m_hDC, "!", 1, &size);
    sizeRet.cx = ((size.cx + m_stFontMetrics.tmOverhang) * nLen);
    sizeRet.cy = (m_stFontMetrics.tmHeight + m_stFontMetrics.tmExternalLeading);
    if (bRelease)
    {
        ReleaseDC(pDC);
    }
    font.DeleteObject();
    return sizeRet;
}

void CMainFrame::OnViewOptions()
{
    m_cfgDlg.ShowWindow(SW_SHOW);
}

void CMainFrame::OnViewDebug()
{
    m_debugDlg.ShowWindow(SW_SHOW);
}
//extern "C"{
void DebugOutput(const char *szFormat, ...)
{
    int nLen;
    char *szData;
    va_list vl;
    va_start(vl, szFormat);
    nLen = _vscprintf(szFormat, vl);
    szData = (char*)malloc(nLen + 1);
    if (szData)
    {
        vsprintf(szData, szFormat, vl);
        EnterCriticalSection(&g_debug_cs);
        g_debug_string.append(szData);
        LeaveCriticalSection(&g_debug_cs);
        free(szData);
    }
    va_end(vl);
}
//}
void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent != m_nTimerID)
    {
        CMDIFrameWnd::OnTimer(nIDEvent);
    }
    else
    {
        EnterCriticalSection(&g_debug_cs);
        if (g_debug_string.length() > 0)
        {
            m_debugDlg.DebugOutput(g_debug_string.c_str());
            g_debug_string.clear();
        }
        LeaveCriticalSection(&g_debug_cs);
    }
}

SDComCfg *CMainFrame::GetConfig()
{
    return m_cfgDlg.GetConfig();
}


void CMainFrame::OnDestroy()
{
    CMDIFrameWnd::OnDestroy();
}

LRESULT CMainFrame::OnFindDialogMessage(WPARAM wParam, LPARAM lParam)
{
    if (m_cFindReplaceDlg->IsTerminating())
    {
        m_cFindReplaceDlg = NULL;
        return 0;
    }
    if (m_cFindReplaceDlg->FindNext())
    {
        m_szFindString = m_cFindReplaceDlg->GetFindString();
        m_bFindMatchCase = m_cFindReplaceDlg->MatchCase();
        m_bFindSearchDown = m_cFindReplaceDlg->SearchDown();
        
        DoFindText(m_bFindSearchDown);
    }
    return 0;
}

void CMainFrame::FindTextResetAll()
{
    POSITION appPos, templatePos;
    CDocTemplate *pTemplate;
    CDComDoc *pDoc;

    appPos = AfxGetApp()->GetFirstDocTemplatePosition();

    while (appPos)
    {
        pTemplate = AfxGetApp()->GetNextDocTemplate(appPos);
        if (pTemplate)
        {
            templatePos = pTemplate->GetFirstDocPosition();
            while (templatePos)
            {
                pDoc = (CDComDoc*)pTemplate->GetNextDoc(templatePos);
                if (pDoc)
                {
                    pDoc->FindTextReset();
                }
            }
        }
    }
}

void CMainFrame::OnEditFind()
{
    DWORD dwFlags;
    m_cFindReplaceDlg = new CFindReplaceDialog;
    dwFlags = FR_HIDEWHOLEWORD;
    if (m_bFindSearchDown == TRUE) 
    {
        dwFlags |= FR_DOWN;
    }
    if (m_bFindMatchCase  == TRUE) 
    {
        dwFlags |= FR_MATCHCASE;
    }
    FindTextResetAll();
    m_cFindReplaceDlg->Create(TRUE, m_szFindString, NULL, dwFlags);
}

void CMainFrame::OnEditRepeat()
{
    DoFindText(TRUE);
}

void CMainFrame::OnEditRepeatUp()
{
    DoFindText(FALSE);
}

void CMainFrame::DoFindText(BOOL bFindSearchDown)
{
    CDComDoc* pDoc = NULL;
    CFrameWnd* pFrame;

    pFrame = ((CMDIFrameWnd*)AfxGetMainWnd())->MDIGetActive();
    if (pFrame)
    {
        pDoc = (CDComDoc*)pFrame->GetActiveDocument();
        if (pDoc)
        {
            pDoc->FindText(m_szFindString, m_bFindMatchCase, bFindSearchDown);
        }
    }
}

void CMainFrame::OnUpdateEditFind(CCmdUI *pCmdUI)
{
    if (MDIGetActive())
    {
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }
}

void CMainFrame::OnUpdateEditRepeat(CCmdUI *pCmdUI)
{
    if (MDIGetActive())
    {
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }
}

void CMainFrame::OnUpdateEditRepeatUp(CCmdUI *pCmdUI)
{
    if (MDIGetActive())
    {
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }
}

BOOL CMainFrame::GetTerminalFile(BOOL bOpen, CString *pszFile)
{
    CString szInitialPath;
    DWORD dwFlags;
    BOOL bRet = FALSE;

    dwFlags = OFN_HIDEREADONLY | OFN_ENABLESIZING;
    if (bOpen == TRUE)
    {
        dwFlags |= OFN_FILEMUSTEXIST;
    }
    CFileDialog fileDlg(bOpen, NULL, NULL, dwFlags, 
        DCOM_TERMINALS_EXTS, this);
    szInitialPath = AfxGetApp()->GetProfileString(DCOM_TERMINALS_SECTION, DCOM_TERMINALS_PATH, "");
    if (szInitialPath.GetLength())
    {
        fileDlg.m_ofn.lpstrInitialDir = szInitialPath;
    }
    if (fileDlg.DoModal() == IDOK)
    {
        *pszFile = fileDlg.GetPathName();
        AfxGetApp()->WriteProfileString(DCOM_TERMINALS_SECTION, DCOM_TERMINALS_PATH, *pszFile);
        bRet = TRUE;
    }
    return bRet;
}

void CMainFrame::OnFileSaveTerminals()
{
    POSITION appPos, templatePos;
    CDocTemplate *pTemplate;
    CDComDoc *pDoc;
    SDialingDir *psDialingDir;
    CString szFile;
    FILE *fTerminalFile;
    
    if (GetTerminalFile(FALSE, &szFile))
    {
        fTerminalFile = fopen(szFile.GetBuffer(), "wb");
        if (fTerminalFile == NULL)
        {
            CString szError;
            szError.Format("Unable to open file %s", szFile);
            AfxMessageBox(szError);
        }
        else
        {
            appPos = AfxGetApp()->GetFirstDocTemplatePosition();

            while (appPos)
            {
                pTemplate = AfxGetApp()->GetNextDocTemplate(appPos);
                if (pTemplate)
                {
                    templatePos = pTemplate->GetFirstDocPosition();
                    while (templatePos)
                    {
                        pDoc = (CDComDoc*)pTemplate->GetNextDoc(templatePos);
                        if (pDoc)
                        {
                            psDialingDir = pDoc->GetDialingDir();
                            fwrite(psDialingDir, sizeof(SDialingDir), 1, fTerminalFile);
                        }
                    }
                }
            }
            fclose(fTerminalFile);
        }
    }
}

void CMainFrame::OpenTerminals(CString szFile)
{
    POSITION dtPos = AfxGetApp()->GetFirstDocTemplatePosition();
    CMyMultiDocTemplate *pDocTemplate;
    SDialingDir sDialingDir;
    FILE *fTerminalFile;
    int nElements;

    fTerminalFile = fopen(szFile.GetBuffer(), "rb");
    if (fTerminalFile == NULL)
    {
        CString szError;
        szError.Format("Unable to open file %s", szFile);
        AfxMessageBox(szError);
    }
    else
    {
        if (dtPos != NULL)
        {
            pDocTemplate = (CMyMultiDocTemplate*)AfxGetApp()->GetNextDocTemplate(dtPos);
            if (pDocTemplate)
            {
                do
                {
                    nElements = fread(&sDialingDir, sizeof(sDialingDir), 1, fTerminalFile);
                    if (nElements == 1)
                    {
                        pDocTemplate->OpenDocumentFile(&sDialingDir);
                    }
                } while (nElements == 1);
            }
        }
        fclose(fTerminalFile);
    }
}

void CMainFrame::OnFileOpenTerminals()
{
    CString szFile;
    if (GetTerminalFile(TRUE, &szFile))
    {
        OpenTerminals(szFile);
    }
}

void CMainFrame::OnHelpHelp()
{
    ShowHelp(IDH_MAIN);
}

