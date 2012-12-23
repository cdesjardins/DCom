// DComView.cpp : implementation of the CDComView class
//

#include "stdafx.h"
#include "DCom.h"

#include "DComDoc.h"
#include "DComView.h"
#include "scrollbuff.h"
#include "childfrm.h"
#include "mainfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDComView

IMPLEMENT_DYNCREATE(CDComView, CScrollView)

BEGIN_MESSAGE_MAP(CDComView, CScrollView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CScrollView::OnFilePrintPreview)
    ON_WM_KEYDOWN()
    ON_WM_KILLFOCUS()
    ON_WM_SETFOCUS()
    ON_WM_VSCROLL()
    ON_WM_ERASEBKGND()
    ON_WM_HSCROLL()
    ON_MESSAGE(WM_DOC_UPDATE, OnDocUpdate)
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_COMMAND(ID_EDIT_COPY, &CDComView::OnEditCopy)
    ON_COMMAND(ID_EDIT_PASTE, &CDComView::OnEditPaste)
    ON_WM_CTLCOLOR()
    ON_WM_TIMER()
    ON_WM_CONTEXTMENU()
    ON_COMMAND(ID_BAUD_115200, &CDComView::OnBaud115200)
    ON_COMMAND(ID_BAUD_57600 , &CDComView::OnBaud57600 )
    ON_COMMAND(ID_BAUD_38400 , &CDComView::OnBaud38400 )
    ON_COMMAND(ID_BAUD_19200 , &CDComView::OnBaud19200 )
    ON_COMMAND(ID_BAUD_14400 , &CDComView::OnBaud14400 )
    ON_COMMAND(ID_BAUD_9600  , &CDComView::OnBaud9600  )
    ON_COMMAND(ID_BAUD_4800  , &CDComView::OnBaud4800  )
    ON_COMMAND(ID_BAUD_2400  , &CDComView::OnBaud2400  )
    ON_COMMAND(ID_BAUD_1200  , &CDComView::OnBaud1200  )
    ON_COMMAND(ID_BAUD_600   , &CDComView::OnBaud600   )
    ON_COMMAND(ID_BAUD_300   , &CDComView::OnBaud300   )
    ON_COMMAND(ID_BAUD_110   , &CDComView::OnBaud110   )
    ON_COMMAND(ID_PARITY_NONE,  &CDComView::OnParityNone)
    ON_COMMAND(ID_PARITY_ODD,   &CDComView::OnParityOdd)
    ON_COMMAND(ID_PARITY_EVEN,  &CDComView::OnParityEven)
    ON_COMMAND(ID_PARITY_MARK,  &CDComView::OnParityMark)
    ON_COMMAND(ID_PARITY_SPACE, &CDComView::OnParitySpace)
    ON_COMMAND(ID_BYTESIZE_8, &CDComView::OnByteSize8   )
    ON_COMMAND(ID_BYTESIZE_7, &CDComView::OnByteSize7   )
    ON_COMMAND(ID_STOPBITS_1, &CDComView::OnStopBits1   )
    ON_COMMAND(ID_STOPBITS_2, &CDComView::OnStopBits2   )
    ON_WM_LBUTTONDBLCLK()
    ON_COMMAND(ID_EDIT_SELECT_ALL, &CDComView::OnEditSelectAll)
    ON_WM_MBUTTONDOWN()
    ON_WM_MBUTTONDBLCLK()
    ON_MESSAGE(WM_UPDATE_STATUS, OnUpdateStatus)
    ON_WM_MOUSELEAVE()
    ON_COMMAND(ID_CONTEXT_SETTITLE, &CDComView::OnContextSettitle)
END_MESSAGE_MAP()

// CDComView construction/destruction

CDComView::CDComView()
{
    InitializeCriticalSectionAndSpinCount(&m_csUserInput, 0x1000);

    m_caretLoc = CPoint(0,0);
    m_bBlockUserin = false;
    m_bMouseIn = false;
    m_bCaretHidden = false;
    m_bLeftButtonDown = false;
    m_bDoubleClickDown = false;
    m_bBlinker = false;
    m_nAttributeBitMask = 0;
}

CDComView::~CDComView()
{
    DeleteCriticalSection(&m_csUserInput);
}

BOOL CDComView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CScrollView::PreCreateWindow(cs);
}

// CDComView drawing

bool CDComView::SetupColors(STextRendition sTextRendition, SDComCfg *pDComCfg, CMyMemDC *pDC)
{
    bool bRet = false;
    COLORREF crText;
    COLORREF crBack;

    if (sTextRendition.crBack == SCROLLBUFF_DEFAULT_CR)
    {
        sTextRendition.crBack = pDComCfg->m_crBack;
    }
    if (sTextRendition.crText == SCROLLBUFF_DEFAULT_CR)
    {
        sTextRendition.crText = pDComCfg->m_crFore;
    }
    
    if (sTextRendition.nAttributes & SB_ATTR_NEGITAVE)
    {
        crText = sTextRendition.crBack;
        crBack = sTextRendition.crText;
    }
    else
    {
        crText = sTextRendition.crText;
        crBack = sTextRendition.crBack;
    }
    if (sTextRendition.nAttributes & SB_ATTR_BOLD)
    {
        crText = SCROLLBUFF_MAKE_BOLD(crText);
    }

    if (crText == crBack)
    {
        crText = (~crText) & 0x00ffffff;
    }
    if (sTextRendition.nAttributes & SB_ATTR_BLINK)
    {
        if (m_bBlinker)
        {
            crText = crBack;
        }
        bRet = true;
    }

    if (!pDC->IsPrinting())
    {
        pDC->SetTextColor(crText);
        pDC->SetBkColor(crBack);
    }
    return bRet;
}

void CDComView::DrawLine(long nLine, SFullLine *sFullLine, SDComCfg *pDComCfg, CMyMemDC* pDC)
{
    int nCol;
    int nOffs;
    STextRendition sTextRendition;
    int nMultiplier = 1;

    if (pDC->IsPrinting())
    {
        nMultiplier = -1;
    }

    memset(&sTextRendition, 0, sizeof(sTextRendition));
    nOffs = 0;
    for (nCol = 0; nCol < SB_NUM_COLS; nCol++)
    {
        if ((sTextRendition != sFullLine->sTextRendition[nCol]) || 
            (sFullLine->szLine[nCol] == 0) ||
            (nCol >= (SB_NUM_COLS - 1)))
        {
            sTextRendition = sFullLine->sTextRendition[nCol];
            pDC->TextOut(
                nOffs * m_sizeCharDimensions.cx, 
                (nLine * m_sizeCharDimensions.cy) * nMultiplier,
                sFullLine->szLine + nOffs, nCol - nOffs);
            nOffs = nCol;
            SetupColors(sTextRendition, pDComCfg, pDC);
            if (sFullLine->szLine[nCol] == 0)
            {
                break;
            }
        }
    }
}

void CDComView::DrawLines(int nFirstLine, int nLastLine, SDComCfg *pDComCfg, CMyMemDC* pDC)
{
    bool bValid;
    SFullLine *sFullLine;
    long nLine;
    CDComDoc *pDoc = GetDocument();

    if (pDoc)
    {
        for (nLine = nFirstLine; nLine < nLastLine; nLine++)
        {
            sFullLine = pDoc->m_cLineBuffer.GetLine(nLine, &bValid);
            if (bValid == true)
            {
                DrawLine(nLine, sFullLine, pDComCfg, pDC);
            }
        }
    }
}

void CDComView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
    CScrollView::OnPrint(pDC, pInfo);
}

void CDComView::OnDraw(CDC* dc)
{
    CRect rcBounds;
	GetClientRect(&rcBounds);
    int nFirstLine;
    int nLastLine;
    CRect rcClient;
    CDComDoc* pDoc = GetDocument();
    CPoint ptScroll;
    SDComCfg *pDComCfg;
    CFont font;
    CMainFrame *pMainFrm = (CMainFrame *)AfxGetMainWnd();

    if (dc->IsPrinting())
    {
        dc->SetMapMode(MM_LOENGLISH);
    }
    else
    {
        dc->SetMapMode(MM_TEXT);
    }
    ASSERT_VALID(pMainFrm);
    if (!pMainFrm)
    {
        return;
    }
    ASSERT_VALID(pDoc);
    if (!pDoc)
    {
        return;
    }

    pDComCfg = pMainFrm->GetConfig();
    if (!dc->IsPrinting())
    {
        dc->SetBkColor(pDComCfg->m_crBack);
    }

    CMyMemDC pDC(dc);

    font.CreateFontIndirect(&pDComCfg->m_logfont);
    pDC->SelectObject(font);
    font.DeleteObject();

    pDoc->m_cLineBuffer.Enter();

    ptScroll = GetDeviceScrollPosition();
    GetClientRect(&rcClient);

    nFirstLine = ptScroll.y / m_sizeCharDimensions.cy;
    nLastLine = nFirstLine + (rcClient.Height() / m_sizeCharDimensions.cy) + 1;

    DrawLines(nFirstLine, nLastLine, pDComCfg, pDC);
    pDoc->m_cLineBuffer.Exit();
}

void CDComView::HideCaret()
{
    if (m_bCaretHidden == false)
    {
        CScrollView::HideCaret();
        m_bCaretHidden = true;
    }
}

void CDComView::ShowCaret() 
{
    if (m_bCaretHidden == true)
    {
        CScrollView::ShowCaret();
        m_bCaretHidden = false;
    }
    if (m_bCaretHidden == false)
    {
        PostMessage(WM_DOC_UPDATE, 0);
    }
}

LRESULT CDComView::OnDocUpdate(WPARAM nNumNewLinesCleared, LPARAM)
{
    CRect rcClient;
    CPoint ptScroll;
    CDComDoc *pDoc;
    CMainFrame *pMainFrm = (CMainFrame *)AfxGetMainWnd();
    if (pMainFrm)
    {
        m_sizeCharDimensions = pMainFrm->GetLineDimentions("a", 1);
    }

    pDoc = GetDocument();
    if (pDoc)
    {
        pDoc->m_cLineBuffer.Enter();
        ptScroll = GetDeviceScrollPosition();

        if ((nNumNewLinesCleared) && (IsScrollBarAtBottom() == false))
        {
            ptScroll.y -= m_sizeCharDimensions.cy * nNumNewLinesCleared;
            ScrollToPosition(ptScroll);
        }
        GetClientRect(&rcClient);

        SetCaretPos(ptScroll, rcClient);
        SetScrollSizes();
        pDoc->m_cLineBuffer.Exit();
    }
    return 0;
}

void CDComView::SetCaretPos(CPoint ptScroll, CRect rcClient)
{
    CDComDoc *pDoc;
    SIZE sizeCaretPos;
    pDoc = GetDocument();
    if (pDoc)
    {
        if (m_bCaretHidden == false)
        {
            sizeCaretPos.cx = pDoc->m_cLineBuffer.GetCaretCol() * m_sizeCharDimensions.cx;
            sizeCaretPos.cy = (pDoc->m_cLineBuffer.GetCaretLine() * m_sizeCharDimensions.cy) - ptScroll.y;
            if (ptScroll.x > 0)
            {
                sizeCaretPos.cx -= 1;
            }
            m_caretLoc = CPoint(sizeCaretPos.cx, sizeCaretPos.cy);
            CView::SetCaretPos(m_caretLoc);
        }
    }
}

int CDComView::GetUserInput(char* szUserInput, int nMaxBytes)
{
    int nRet = 0;
    EnterCriticalSection(&m_csUserInput);
    nRet = Vec2Char(szUserInput, &m_strUserInput, nMaxBytes);
    LeaveCriticalSection(&m_csUserInput);
    return nRet;
}

void CDComView::OnInitialUpdate()
{
    CDComDoc *pDoc;
	CScrollView::OnInitialUpdate();
    CDC *pDC;
    pDC = GetDC();
    m_crBack = pDC->GetBkColor();
    m_crText = pDC->GetTextColor();
    ReleaseDC(pDC);
    pDoc = GetDocument();
    if (pDoc)
    {
        pDoc->SetTitle();
        CMainFrame *pMainFrm = (CMainFrame *)AfxGetMainWnd();
        if (pMainFrm)
        {
            m_sizeCharDimensions = pMainFrm->GetLineDimentions("a", 1);
        }
    }
    OnDocUpdate(0,0);
    m_nTimerID = SetTimer(1, 1000, NULL);
}

bool CDComView::IsScrollBarAtBottom()
{
    BOOL bVert, bHorz;
    CheckScrollBars(bHorz, bVert);
    if ((GetScrollPos(SB_VERT) >= GetScrollLimit(SB_VERT)) || (bVert == false))
    {
        return true;
    }
    return false;
}

void CDComView::SetScrollSizes()
{
    SIZE sizeTotal;
    SIZE sizePage;
    CDComDoc *pDoc;
    bool bBottom = false;

    pDoc = GetDocument();
    if (pDoc)
    {
        sizeTotal.cx = 0;
        sizeTotal.cy = pDoc->m_cLineBuffer.GetNumLines() * m_sizeCharDimensions.cy;
        bBottom = IsScrollBarAtBottom();
        sizePage.cx = m_sizeCharDimensions.cx * 25;
        sizePage.cy = m_sizeCharDimensions.cy * 25;
        CScrollView::SetScrollSizes(MM_TEXT, sizeTotal, sizePage, m_sizeCharDimensions);
        if (bBottom == true)
        {
            PostMessage(WM_VSCROLL, SB_BOTTOM);
        }
    }
}

BOOL CDComView::OnScrollBy(CSize sizeScroll, BOOL bDoScroll)
{
    return CScrollView::OnScrollBy(sizeScroll, bDoScroll);
}

BOOL CDComView::OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll)
{
    SCROLLINFO info;
    if (HIBYTE(nScrollCode) == SB_THUMBTRACK)
    {
        GetScrollInfo(SB_VERT, &info);
        nPos = info.nTrackPos;
        Invalidate();
    }
    return CScrollView::OnScroll(nScrollCode, nPos, bDoScroll);
}

BOOL CDComView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CDComView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CDComView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// CDComView diagnostics

#ifdef _DEBUG
void CDComView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CDComView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CDComDoc* CDComView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDComDoc)));
	return (CDComDoc*)m_pDocument;
}
#endif //_DEBUG


void CDComView::BlockUserInput(bool bBlockUserin)
{
    m_bBlockUserin = bBlockUserin;
}

void CDComView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    WORD wCh;
    char szInput[2];
    if (m_bBlockUserin == false)
    {
        memset(m_kbState, 0, sizeof(m_kbState));
        GetKeyboardState(m_kbState);
        if ((ToAscii(nChar, MapVirtualKey(nChar, MAPVK_VK_TO_CHAR), m_kbState, &wCh, 0) == 1) &&
            ((myisprint((char)wCh)) || ((char)wCh == VK_ESCAPE) || (GetKeyState(VK_CONTROL) & 0x8000)))
        {
            szInput[0] = (char)wCh;
            szInput[1] = 0;
            AddUserInput(szInput, 1);
        }
        else
        {
            CMainFrame *pMainFrm = (CMainFrame *)AfxGetMainWnd();
            char *pText;
            if (pMainFrm)
            {
                pText = pMainFrm->GetKeyString(nChar, 
                    (GetKeyState(VK_CONTROL) & 0x8000) ? true : false, 
                    (GetKeyState(VK_SHIFT  ) & 0x8000) ? true : false);
                if (pText)
                {
                    AddUserInput(pText, strlen(pText));
                }
            }
        }
    }
    CScrollView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CDComView::AddUserInput(char *pInput, int nBytes)
{
    int nIndex;
    EnterCriticalSection(&m_csUserInput);
    for (nIndex = 0; nIndex < nBytes; nIndex++)
    {
        m_strUserInput.push_back(pInput[nIndex]);
    }
    LeaveCriticalSection(&m_csUserInput);
}

void CDComView::OnKillFocus(CWnd* pNewWnd)
{
    CScrollView::OnKillFocus(pNewWnd);

    HideCaret();
}

void CDComView::OnSetFocus(CWnd* pOldWnd)
{
    CMainFrame *pMainFrm = (CMainFrame *)AfxGetMainWnd();
    CScrollView::OnSetFocus(pOldWnd);
    if (pMainFrm)
    {
        CreateSolidCaret(1, pMainFrm->GetLineDimentions("a", 1).cy);
    }
    ShowCaret();
}

void CDComView::UpdateSelectionRange(CPoint point, bool bResetRange, bool bDoubleClickDown)
{
    CDComDoc *pDoc;
    CPoint ptScroll;
    SSelection sSelection;
    ptScroll = GetDeviceScrollPosition();

    pDoc = GetDocument();
    if (pDoc)
    {
        sSelection.x = (point.x + ptScroll.x) / m_sizeCharDimensions.cx;
        sSelection.y = (point.y + ptScroll.y) / m_sizeCharDimensions.cy;

        if (bResetRange)
        {
            pDoc->m_cLineBuffer.ResetSelectionRange(sSelection, bDoubleClickDown);
        }
        else
        {
            pDoc->m_cLineBuffer.SetSelectionRange(sSelection);
        }
    }
}

void CDComView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    m_bDoubleClickDown = true;
    UpdateSelectionRange(point, true, m_bDoubleClickDown);
    m_bLeftButtonDown = true;
    Invalidate();
    CScrollView::OnLButtonDblClk(nFlags, point);
}

void CDComView::OnLButtonDown(UINT nFlags, CPoint point)
{
    if (GetKeyState(VK_SHIFT) & 0x8000)
    {
        UpdateSelectionRange(point, false, m_bDoubleClickDown);
    }
    else
    {
        UpdateSelectionRange(point, true, m_bDoubleClickDown);
    }
    m_bLeftButtonDown = true;
    Invalidate();
    CScrollView::OnLButtonDown(nFlags, point);
}

void CDComView::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (m_bLeftButtonDown == true)
    {
        UpdateSelectionRange(point, false, m_bDoubleClickDown);
        m_bDoubleClickDown = false;
        m_bLeftButtonDown = false;
        PostMessage(WM_COMMAND, MAKEWPARAM(ID_EDIT_COPY, 0));
        Invalidate();
    }
    CScrollView::OnLButtonUp(nFlags, point);
}

void CDComView::OnMouseMove(UINT nFlags, CPoint point)
{
    TRACKMOUSEEVENT tme;
    tme.cbSize = sizeof(tme);
    tme.hwndTrack = m_hWnd;
    tme.dwFlags = TME_LEAVE | TME_HOVER;
    tme.dwHoverTime = 1;
    TrackMouseEvent(&tme);

    if (m_bMouseIn == false)
    {
        m_bMouseIn = true;
        if ((m_bLeftButtonDown == true) && ((MK_LBUTTON & nFlags) != MK_LBUTTON))
        {
            m_bDoubleClickDown = false;
            m_bLeftButtonDown = false;
            PostMessage(WM_COMMAND, MAKEWPARAM(ID_EDIT_COPY, 0));
        }
    }
    if (m_bLeftButtonDown == true)
    {
        UpdateSelectionRange(point, false, m_bDoubleClickDown);
        Invalidate();
    }
    CScrollView::OnMouseMove(nFlags, point);
}

void CDComView::OnMouseLeave()
{
    if (m_bLeftButtonDown == true)
    {
        PostMessage(WM_COMMAND, MAKEWPARAM(ID_EDIT_COPY, 0));
    }
    m_bMouseIn = false;
    CScrollView::OnMouseLeave();
}

void CDComView::OnMButtonDown(UINT nFlags, CPoint point)
{
    PostMessage(WM_COMMAND, MAKEWPARAM(ID_EDIT_PASTE, 0));
}

void CDComView::OnMButtonDblClk(UINT nFlags, CPoint point)
{
    PostMessage(WM_COMMAND, MAKEWPARAM(ID_EDIT_PASTE, 0));
}

void CDComView::OnEditSelectAll()
{
    CDComDoc *pDoc;
    SSelection sSelection;

    pDoc = GetDocument();
    if (pDoc)
    {
        sSelection.x = 0;
        sSelection.y = 0;
        pDoc->m_cLineBuffer.ResetSelectionRange(sSelection, false);
        sSelection.x = SB_NUM_COLS;
        sSelection.y = pDoc->m_cLineBuffer.GetNumLines() - 1;
        pDoc->m_cLineBuffer.SetSelectionRange(sSelection);
        Invalidate();
        PostMessage(WM_COMMAND, MAKEWPARAM(ID_EDIT_COPY, 0));
    }
}

void CDComView::OnEditCopy()
{
    std::string szText;
    CDComDoc *pDoc;
    HANDLE hClipboard;
    LPTSTR lptstrCopy;

    pDoc = GetDocument();
    if (pDoc)
    {
        pDoc->m_cLineBuffer.Enter();
        pDoc->m_cLineBuffer.GetSelectedText(&szText);
        pDoc->m_cLineBuffer.Exit();
        if (szText.length() > 0)
        {
            OpenClipboard();
            EmptyClipboard();
            hClipboard = GlobalAlloc(GPTR, szText.size() + 1);
            lptstrCopy = (LPTSTR)GlobalLock(hClipboard);
            memcpy(lptstrCopy, szText.c_str(), szText.size());
            GlobalUnlock(hClipboard);
            SetClipboardData(CF_TEXT, hClipboard);
            CloseClipboard();
        }
    }
}

void CDComView::OnEditPaste()
{
    char *pBuffer;
    HANDLE hClipboard;

    if (OpenClipboard())
    {
        hClipboard = GetClipboardData(CF_TEXT);
        pBuffer = (char*)GlobalLock(hClipboard);
        AddUserInput(pBuffer, strlen(pBuffer));
        GlobalUnlock(hClipboard);
        CloseClipboard();
    }
}

void CDComView::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent != m_nTimerID)
    {
        CScrollView::OnTimer(nIDEvent);
    }
    else
    {
        m_bBlinker = !m_bBlinker;
        Invalidate();
    }
}

BOOL CDComView::PreTranslateMessage(MSG* pMsg)
{
    if ((pMsg->message == WM_SYSKEYDOWN) && (pMsg->wParam == VK_F10))
    {
        pMsg->message = WM_KEYDOWN;
    }
    return CScrollView::PreTranslateMessage(pMsg);
}

void CDComView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
    CDComDoc *pDoc;
    pDoc = GetDocument();
    if (pDoc)
    {
        if (pDoc->m_nContextMenuId == IDR_SERIAL_CONTEXT)
        {
            DoSerialMenu(pDoc, point);
        }
    }
}


SMyMap g_baudMap[] = 
{
    { ID_BAUD_115200, 115200 },
    { ID_BAUD_57600 , 57600  },
    { ID_BAUD_38400 , 38400  },
    { ID_BAUD_19200 , 19200  },
    { ID_BAUD_14400 , 14400  },
    { ID_BAUD_9600  , 9600   },
    { ID_BAUD_4800  , 4800   },
    { ID_BAUD_2400  , 2400   },
    { ID_BAUD_1200  , 1200   },
    { ID_BAUD_600   , 600    },
    { ID_BAUD_300   , 300    },
    { ID_BAUD_110   , 110    },
};

SMyMap g_parityMap[] = 
{
    { ID_PARITY_NONE,  NOPARITY    },
    { ID_PARITY_ODD,   ODDPARITY   },
    { ID_PARITY_EVEN,  EVENPARITY  },
    { ID_PARITY_MARK,  MARKPARITY  },
    { ID_PARITY_SPACE, SPACEPARITY },
};

SMyMap g_byteSizeMap[] = 
{
    { ID_BYTESIZE_8,  8   },
    { ID_BYTESIZE_7,  7   },
};

SMyMap g_stopBitsMap[] = 
{
    { ID_STOPBITS_1,  ONESTOPBIT  },
    { ID_STOPBITS_2,  TWOSTOPBITS },
};

void CDComView::CheckMenuItems(CMenu *pMenu, SMyMap* pMap, int nMapSize, int nCheckVal)
{
    int i;
    for (i = 0; i < nMapSize; i++)
    {
        if (pMap[i].nVal == nCheckVal)
        {
            pMenu->CheckMenuItem(pMap[i].nId, MF_CHECKED);
        }
        else
        {
            pMenu->CheckMenuItem(pMap[i].nId, MF_UNCHECKED);
        }
    }
}

void CDComView::DoSerialMenu(CDComDoc *pDoc, CPoint point)
{
    TgtSerialIntf::TgtConnection sTgtConfig;
    TgtSerialIntf *pTgtIntf;
    CMenu mnuContext;
    CMenu *mnuPopupMenu;
    CMenu *mnuSubmenu;
 
    mnuContext.LoadMenu(IDR_SERIAL_CONTEXT);
    mnuPopupMenu = mnuContext.GetSubMenu(0);

    pTgtIntf = (TgtSerialIntf*)pDoc->m_pTgtIntf;

    sTgtConfig = pTgtIntf->TgtGetConfig();

    mnuSubmenu = mnuPopupMenu->GetSubMenu(0);
    CheckMenuItems(mnuSubmenu, g_baudMap, sizeof(g_baudMap) / sizeof(SMyMap), sTgtConfig.m_dwBaudRate);

    mnuSubmenu = mnuPopupMenu->GetSubMenu(1);
    CheckMenuItems(mnuSubmenu, g_parityMap, sizeof(g_parityMap) / sizeof(SMyMap), sTgtConfig.m_byParity);

    mnuSubmenu = mnuPopupMenu->GetSubMenu(2);
    CheckMenuItems(mnuSubmenu, g_byteSizeMap, sizeof(g_byteSizeMap) / sizeof(SMyMap), sTgtConfig.m_byByteSize);

    mnuSubmenu = mnuPopupMenu->GetSubMenu(3);
    CheckMenuItems(mnuSubmenu, g_stopBitsMap, sizeof(g_stopBitsMap) / sizeof(SMyMap), sTgtConfig.m_byStopBits);

    mnuPopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
}

void CDComView::OnBaud(int nRate)
{
    TgtSerialIntf::TgtConnection sTgtConfig;
    CDComDoc *pDoc;
    TgtSerialIntf *pTgtIntf;
    char *pErr;

    pDoc = GetDocument();
    if (pDoc)
    {
        pTgtIntf = (TgtSerialIntf*)pDoc->m_pTgtIntf;
        sTgtConfig = pTgtIntf->TgtGetConfig();
        sTgtConfig.m_dwBaudRate = nRate;
        pTgtIntf->TgtSetConfig(&sTgtConfig);
        pErr = pTgtIntf->TgtSetupPort();
        if (pErr)
        {
            AfxMessageBox(pErr);
        }
        pDoc->SetTitle();
    }
}

void CDComView::OnBaud115200()
{
    OnBaud(115200);
}
void CDComView::OnBaud57600()
{
    OnBaud(57600);
}
void CDComView::OnBaud38400()
{
    OnBaud(38400);
}
void CDComView::OnBaud19200()
{
    OnBaud(19200);
}
void CDComView::OnBaud14400()
{
    OnBaud(14400);
}
void CDComView::OnBaud9600()
{
    OnBaud(9600);
}
void CDComView::OnBaud4800()
{
    OnBaud(4800);
}
void CDComView::OnBaud2400()
{
    OnBaud(2400);
}
void CDComView::OnBaud1200()
{
    OnBaud(1200);
}
void CDComView::OnBaud600()
{
    OnBaud(600);
}
void CDComView::OnBaud300()
{
    OnBaud(300);
}
void CDComView::OnBaud110()
{
    OnBaud(110);
}

void CDComView::OnParity(int nParity)
{
    TgtSerialIntf::TgtConnection sTgtConfig;
    CDComDoc *pDoc;
    TgtSerialIntf *pTgtIntf;
    char *pErr;

    pDoc = GetDocument();
    if (pDoc)
    {
        pTgtIntf = (TgtSerialIntf*)pDoc->m_pTgtIntf;
        sTgtConfig = pTgtIntf->TgtGetConfig();
        sTgtConfig.m_byParity = nParity;
        pTgtIntf->TgtSetConfig(&sTgtConfig);
        pErr = pTgtIntf->TgtSetupPort();
        if (pErr)
        {
            AfxMessageBox(pErr);
        }
        pDoc->SetTitle();
    }
}
void CDComView::OnParityNone()
{
    OnParity(NOPARITY);
}
void CDComView::OnParityOdd()
{
    OnParity(ODDPARITY);
}
void CDComView::OnParityEven()
{
    OnParity(EVENPARITY);
}
void CDComView::OnParityMark()
{
    OnParity(MARKPARITY);
}
void CDComView::OnParitySpace()
{
    OnParity(SPACEPARITY);
}

void CDComView::OnByteSize(int nByteSize)
{
    TgtSerialIntf::TgtConnection sTgtConfig;
    CDComDoc *pDoc;
    TgtSerialIntf *pTgtIntf;
    char *pErr;

    pDoc = GetDocument();
    if (pDoc)
    {
        pTgtIntf = (TgtSerialIntf*)pDoc->m_pTgtIntf;
        sTgtConfig = pTgtIntf->TgtGetConfig();
        sTgtConfig.m_byByteSize = nByteSize;
        pTgtIntf->TgtSetConfig(&sTgtConfig);
        pErr = pTgtIntf->TgtSetupPort();
        if (pErr)
        {
            AfxMessageBox(pErr);
        }
        pDoc->SetTitle();
    }
}
void CDComView::OnByteSize8()
{
    OnByteSize(8);
}
void CDComView::OnByteSize7()
{
    OnByteSize(7);
}

void CDComView::OnStopBits(int nStopBits)
{
    TgtSerialIntf::TgtConnection sTgtConfig;
    CDComDoc *pDoc;
    TgtSerialIntf *pTgtIntf;
    char *pErr;

    pDoc = GetDocument();
    if (pDoc)
    {
        pTgtIntf = (TgtSerialIntf*)pDoc->m_pTgtIntf;
        sTgtConfig = pTgtIntf->TgtGetConfig();
        sTgtConfig.m_byStopBits = nStopBits;
        pTgtIntf->TgtSetConfig(&sTgtConfig);
        pErr = pTgtIntf->TgtSetupPort();
        if (pErr)
        {
            AfxMessageBox(pErr);
        }
        pDoc->SetTitle();
    }
}

void CDComView::OnStopBits1()
{
    OnStopBits(ONESTOPBIT);
}
void CDComView::OnStopBits2()
{
    OnStopBits(TWOSTOPBITS);
}

LRESULT CDComView::OnUpdateStatus(WPARAM, LPARAM)
{
    CChildFrame *pFrame;

    pFrame = (CChildFrame*)GetParent();
    if (pFrame)
    {
        pFrame->UpdateStatusPane(m_szStatus);
    }
    return 0;
}

void CDComView::OnContextSettitle()
{
    CDComDoc *pDoc;
    pDoc = GetDocument();
    if (pDoc)
    {
        pDoc->ExtendWindowTitle();
    }
}
