// DComDoc.cpp : implementation of the CDComDoc class
//

#include "stdafx.h"
#include "DCom.h"
#include "MainFrm.h"
#include "DComDoc.h"
#include "ChildFrm.h"
#include "TitleDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CD ComDoc

IMPLEMENT_DYNCREATE(CDComDoc, CDocument)

BEGIN_MESSAGE_MAP(CDComDoc, CDocument)
END_MESSAGE_MAP()


// CDComDoc construction/destruction

CDComDoc::CDComDoc()
{
    m_nContextMenuId = 0;
	m_pTgtIntf = 0;
    m_cTgtIntfThread = 0;
    m_bFileCapture = false;
    m_hThreadTerm = 0;
    FindTextReset();
}

CDComDoc::~CDComDoc()
{
    if (m_pTgtIntf)
    {
        delete (m_pTgtIntf);
    }
}

BOOL CDComDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
    {
        return FALSE;
    }

    // TODO: add reinitialization code here
    // (SDI documents will reuse this document)

    return TRUE;
}




// CDComDoc serialization

void CDComDoc::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        // TODO: add storing code here
    }
    else
    {
        // TODO: add loading code here
    }
}


// CDComDoc diagnostics

#ifdef _DEBUG
void CDComDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CDComDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

void CDComDoc::InjectAnsi(CDComDoc *pDoc)
{
    /*
#define TEST_ANSI "\033[32m1234567890\033[1D\033[1D\033[1D\033[2P"
    pDoc->m_cLineBuffer.AddText(TEST_ANSI, strlen(TEST_ANSI));
    */
}

UINT CDComDoc::TgtIntfThread(LPVOID arg)
{
    CDComDoc *pDoc;
    CDComView *pView;
    int nNumBytes;
    long nNumNewLinesCleared = 0;

    pDoc = (CDComDoc*)arg;
    pView = pDoc->GetView();

    pDoc->InjectAnsi(pDoc);
    do
    {
        nNumBytes = pDoc->m_pTgtIntf->TgtRead(pDoc->m_sTgtIOBuffer, sizeof(pDoc->m_sTgtIOBuffer) - 1);
        if (nNumBytes > 0)
        {
            pDoc->m_sTgtIOBuffer[nNumBytes] = 0;
            pView->SetRedraw(FALSE);
            nNumNewLinesCleared = pDoc->m_cLineBuffer.AddText(pDoc->m_sTgtIOBuffer, nNumBytes);
            pDoc->m_cCmdRedirector.PrintS(pDoc->m_sTgtIOBuffer, nNumBytes);
            pView->PostMessage(WM_DOC_UPDATE, nNumNewLinesCleared);
            pView->SetRedraw(TRUE);
            pView->Invalidate();
        }
        nNumBytes = pView->GetUserInput(pDoc->m_sTgtIOBuffer, sizeof(pDoc->m_sTgtIOBuffer));
        if (nNumBytes > 0)
        {
            pDoc->m_pTgtIntf->TgtWrite(pDoc->m_sTgtIOBuffer, nNumBytes);
        }
        if (pDoc->m_pTgtIntf->TgtConnected() == false)
        {
            break;
        }
    } while (WaitForSingleObject(pDoc->m_hThreadTerm, 1) != WAIT_OBJECT_0);
    SetEvent(pDoc->m_hThreadTerminated);
    return 3;
}

CDComView *CDComDoc::GetView()
{
    POSITION viewPos;
    CDComView *pView;
    viewPos = GetFirstViewPosition();
    pView = (CDComView*)GetNextView(viewPos);
    return pView;
}

BOOL CDComDoc::RunCommand(LPCTSTR lpszCommand, LPCTSTR lpszDirectory, bool bStdin, bool bPipein, bool bUserin)
{
    CDComView *pView;

    m_cCmdRedirector.Close();
    pView = GetView();
    if (pView)
    {
        pView->BlockUserInput(bUserin);
    }
    return m_cCmdRedirector.Open(lpszCommand, lpszDirectory, this, bStdin, bPipein);
}


BOOL CDComDoc::SaveModified()
{
    return TRUE;
}

void CDComDoc::SetTitle()
{
    CString szTmp;
    char szTitle[128];
    m_pTgtIntf->TgtGetTitle(szTitle);
    if (m_szExtendedTitle.GetLength() > 0)
    {
        szTmp.Format("%s - %s", m_szExtendedTitle.GetBuffer(), szTitle);
    }
    else
    {
        szTmp.Format("%s", szTitle);
    }
    CDocument::SetTitle(szTmp);
}

void CDComDoc::ExtendWindowTitle()
{
    CTitleDlg titleDlg;
    if (m_szExtendedTitle.GetLength() > 0)
    {
        titleDlg.m_szTitleText = m_szExtendedTitle;
    }
    if (titleDlg.DoModal() == IDOK)
    {
        m_szExtendedTitle = titleDlg.m_szTitleText;
        SetTitle();
    }
}

void CDComDoc::ClearScreen()
{
    CDComView *pView;

    pView = GetView();
    if (pView)
    {
        pView->SetRedraw(FALSE);
        m_cLineBuffer.ClearAllLines();
        pView->PostMessage(WM_DOC_UPDATE, 0);
        pView->SetRedraw(TRUE);
        pView->Invalidate();
    }
}

BOOL CDComDoc::OnOpenDocument(SDialingDir *psDialingDir)
{
    char *pRet;
    BOOL bRet = FALSE;
    CMainFrame *pMainFrm = (CMainFrame *)AfxGetMainWnd();

    if (pMainFrm == NULL)
    {
        return FALSE;
    }
    memcpy(&m_sDialingDir, psDialingDir, sizeof(SDialingDir));

    if (m_sDialingDir.nType == DCOM_DD_SERIAL)
    {
        m_nContextMenuId = IDR_SERIAL_CONTEXT;
        m_pTgtIntf = new TgtSerialIntf;
        if (m_pTgtIntf)
        {
            ((TgtSerialIntf*)m_pTgtIntf)->TgtSetConfig(
                m_sDialingDir.szAddress,
                m_sDialingDir.m_dwBaudRate,
                m_sDialingDir.m_byParity,
                m_sDialingDir.m_byStopBits,
                m_sDialingDir.m_byByteSize);
        }
    }
    else if (m_sDialingDir.nType == DCOM_DD_SSH)
    {
        m_pTgtIntf = new TgtSshIntf;
        if (m_pTgtIntf)
        {
            ((TgtSshIntf*)m_pTgtIntf)->TgtSetConfig(
                m_sDialingDir.szAddress,
                m_sDialingDir.nPort,
                m_sDialingDir.szDescription);
        }
    }
    else if (m_sDialingDir.nType == DCOM_DD_TELNET)
    {
        m_pTgtIntf = new TgtTelnetIntf;
        if (m_pTgtIntf)
        {
            ((TgtTelnetIntf*)m_pTgtIntf)->TgtSetConfig(
                m_sDialingDir.szAddress,
                m_sDialingDir.nPort,
                m_sDialingDir.szDescription);
        }
    }
    else if (m_sDialingDir.nType == DCOM_DD_DEBUG)
    {
        m_pTgtIntf = new TgtFileIntf;
        if (m_pTgtIntf)
        {
            ((TgtFileIntf*)m_pTgtIntf)->TgtSetConfig(
                m_sDialingDir.szAddress);
        }
    }

    if (m_pTgtIntf)
    {
        SetConfig(pMainFrm->GetConfig());
        pRet = m_pTgtIntf->TgtConnect();
        if (pRet == NULL)
        {
            m_hThreadTerminated = CreateEvent(0,0,0,0);
            m_hThreadTerm = CreateEvent(0,0,0,0);
            m_cTgtIntfThread = AfxBeginThread(&CDComDoc::TgtIntfThread, (LPVOID)this, THREAD_PRIORITY_IDLE);
            bRet = TRUE;
        }
        else
        {
            AfxMessageBox(pRet);
        }
    }
    return bRet;
}

void CDComDoc::SetConfig(SDComCfg *sDComCfg)
{
    m_cLineBuffer.SetEmulation(sDComCfg->m_nEmulation);
    m_cLineBuffer.SetMaxNumLines(sDComCfg->m_nNumLines);
    m_cLineBuffer.SetLineWidth(sDComCfg->m_nWinWidth + SB_MIN_COLS);
}

void CDComDoc::OnCloseDocument()
{
    if (m_hThreadTerm)
    {
        SetEvent(m_hThreadTerm);
        WaitForSingleObject(m_hThreadTerminated, 1000);
        CloseHandle(m_hThreadTerm);
        CloseHandle(m_hThreadTerminated);
    }
    if (m_pTgtIntf)
    {
        m_pTgtIntf->TgtDisconnect();
    }

    CDocument::OnCloseDocument();
}

void CDComDoc::OnFileCapture(CWnd *pParent)
{

    if (m_bFileCapture == true) 
    {
        m_bFileCapture = false;
        m_cLineBuffer.CaptureStop();
    }
    else
    {
        m_bFileCapture = true;
        CFileDialog fileDlg(FALSE, NULL, NULL, 
            OFN_HIDEREADONLY | OFN_ENABLESIZING, 
            "Capture Files (*.cap)|*.cap|All Files (*.*)|*.*||", pParent);

        fileDlg.m_ofn.lpstrTitle = GetTitle();
        if (fileDlg.DoModal() == IDOK)
        {
            m_cLineBuffer.CaptureStart(fileDlg.GetPathName().GetBuffer());
        }
    }
}

void CDComDoc::FindText(CString szFindString, BOOL bFindMatchCase, BOOL bFindSearchDown)
{
    CDComView *pView;
    CMainFrame *pMainFrm;
    SIZE charDimentions;
    if (szFindString.GetLength() > 0)
    {
        pView = GetView();
        if (m_cLineBuffer.FindText(szFindString, bFindMatchCase, bFindSearchDown, &m_sFindStartingLine) == false)
        {
            FindTextReset();
        }
        else
        {
            pMainFrm = (CMainFrame *)AfxGetMainWnd();
            if (pMainFrm)
            {
                charDimentions = pMainFrm->GetLineDimentions("a", 1);
            }
            else
            {
                charDimentions.cx = 1;
                charDimentions.cy = 1;
            }
            pView->ScrollToPosition(CPoint(
                0, m_sFindStartingLine.y * charDimentions.cy));
            pView->Invalidate();
        }
    }
}

void CDComDoc::FindTextReset()
{
    m_sFindStartingLine.x = 0;
    m_sFindStartingLine.y = -1;
}

CMyRedirector::CMyRedirector() 
: m_pDoc(NULL)
{
}

void CMyRedirector::WriteStdOut(LPCSTR pszOutput, DWORD dwChars)
{
    CDComView *pView;
    if (m_pDoc)
    {
        pView = m_pDoc->GetView();
        if (pView)
        {
            pView->AddUserInput((char*)pszOutput, dwChars);
        }
    }
}

BOOL CMyRedirector::Open(LPCTSTR pszCmdLine, LPCTSTR lpszDirectory, CDComDoc* pDoc, bool bStdin, bool bPipein)
{
    CDComView *pView;

    m_pDoc = pDoc;
    if (m_pDoc)
    {
        pView = m_pDoc->GetView();
        if (pView)
        {
            pView->m_szStatus.Format("Running command: %s", pszCmdLine);
            pView->PostMessage(WM_UPDATE_STATUS);
        }
    }
    return CRedirector::Open(pszCmdLine, lpszDirectory, bStdin, bPipein);
}

void CMyRedirector::Close()
{
    CDComView *pView;
    CRedirector::Close();
    if (m_pDoc)
    {
        pView = m_pDoc->GetView();
        if (pView)
        {
            pView->m_szStatus.Format("");
            pView->PostMessage(WM_UPDATE_STATUS);
            pView->BlockUserInput(false);
        }
    }
}

BOOL CMyRedirector::IsRunning()
{
    return m_dwThreadId != 0;
}

CDocument* CMyMultiDocTemplate::OpenDocumentFile(SDialingDir *psDialingDir)
{
    CDComDoc* pDocument = (CDComDoc*)CreateNewDocument();
    if (pDocument == NULL)
    {
        TRACE(traceAppMsg, 0, "CDocTemplate::CreateNewDocument returned NULL.\n");
        AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);
        return NULL;
    }
    ASSERT_VALID(pDocument);

    BOOL bAutoDelete = pDocument->m_bAutoDelete;
    pDocument->m_bAutoDelete = FALSE;   // don't destroy if something goes wrong
    CFrameWnd* pFrame = CreateNewFrame(pDocument, NULL);
    pDocument->m_bAutoDelete = bAutoDelete;
    if (pFrame == NULL)
    {
        AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);
        delete pDocument;       // explicit delete on error
        return NULL;
    }
    ASSERT_VALID(pFrame);

    // open an existing document
    CWaitCursor wait;
    if (!pDocument->OnOpenDocument(psDialingDir))
    {
        // user has be alerted to what failed in OnOpenDocument
        TRACE(traceAppMsg, 0, "CDocument::OnOpenDocument returned FALSE.\n");
        pFrame->DestroyWindow();
        return NULL;
    }

    InitialUpdateFrame(pFrame, pDocument, TRUE);
    return pDocument;
}


