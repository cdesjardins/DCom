// DComView.h : interface of the CDComView class
//


#pragma once

#include "scrollbuff.h"
#include "ConfigDlg.h"
#include "MemDC.h"
#include <list>
class CDComDoc;

#define WM_DOC_UPDATE    (WM_USER + 10)
#define WM_UPDATE_STATUS (WM_USER + 11)

struct SMyMap
{
    int nId;
    int nVal;
};

class CDComView;

class CDComView : public CScrollView
{
protected:
	CDComView();
	DECLARE_DYNCREATE(CDComView)
	DECLARE_MESSAGE_MAP()
    std::list<char> m_strUserInput;
    bool m_bBlockUserin;
    bool m_bCaretHidden;
    BYTE m_kbState[256];
    CPoint m_caretLoc;
    bool m_bMouseIn;
    bool m_bLeftButtonDown;
    bool m_bDoubleClickDown;
    CRITICAL_SECTION m_csUserInput;
    SIZE m_sizeCharDimensions;
    COLORREF m_crBack;
    COLORREF m_crText;
    UINT32 m_nAttributeBitMask;
    bool m_bBlinker;
    UINT m_nTimerID;

    afx_msg BOOL OnEraseBkgnd(CDC* pDC) {return FALSE; };
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
    virtual bool SetupColors(STextRendition sTextRendition, SDComCfg *pDComCfg, CMyMemDC *pDC);
    virtual BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll = TRUE);
    virtual void DrawLines(int nFirstLine, int nLastLine, SDComCfg *pDComCfg, CMyMemDC * pDC);
    virtual void DrawLine(long nLine, SFullLine *sFullLine, SDComCfg *pDComCfg, CMyMemDC* pDC);
    virtual LRESULT OnDocUpdate(WPARAM nNumNewLinesCleared, LPARAM);
    virtual LRESULT OnUpdateStatus(WPARAM, LPARAM);
    virtual bool IsScrollBarAtBottom();
    virtual void UpdateSelectionRange(CPoint point, bool bResetRange, bool bDoubleClickDown);
    virtual void DoSerialMenu(CDComDoc *pDoc, CPoint point);
    virtual void CheckMenuItems(CMenu *pMenu, SMyMap* pMap, int nMapSize, int nCheckVal);
    virtual void OnBaud(int nRate);
    virtual void OnParity(int nParity);
    virtual void OnByteSize(int nByteSize);
    virtual void OnStopBits(int nStopBits);
    virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);

public:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

    CDComDoc* GetDocument() const;
	virtual void OnDraw(CDC* dc);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual ~CDComView();
    virtual void BlockUserInput(bool bBlockUserin);
    virtual int GetUserInput(char* szUserInput, int nMaxBytes);
    virtual void SetScrollSizes();
    virtual void SetCaretPos(CPoint ptScroll, CRect rcClient);
    virtual void AddUserInput(char *pInput, int nBytes);
    virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    void HideCaret();
    void ShowCaret();

    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnEditCopy();
    afx_msg void OnEditPaste();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnEditSelectAll();
    afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint point);

    afx_msg void OnBaud115200();
    afx_msg void OnBaud57600 ();
    afx_msg void OnBaud38400 ();
    afx_msg void OnBaud19200 ();
    afx_msg void OnBaud14400 ();
    afx_msg void OnBaud9600  ();
    afx_msg void OnBaud4800  ();
    afx_msg void OnBaud2400  ();
    afx_msg void OnBaud1200  ();
    afx_msg void OnBaud600   ();
    afx_msg void OnBaud300   ();
    afx_msg void OnBaud110   ();

    afx_msg void OnParityNone   ();
    afx_msg void OnParityOdd    ();
    afx_msg void OnParityEven   ();
    afx_msg void OnParityMark   ();
    afx_msg void OnParitySpace  ();

    afx_msg void OnByteSize8    ();
    afx_msg void OnByteSize7    ();

    afx_msg void OnStopBits1    ();
    afx_msg void OnStopBits2    ();

    CString m_szStatus;

    afx_msg void OnMouseLeave();
    afx_msg void OnContextSettitle();
};

#ifndef _DEBUG  // debug version in DComView.cpp
inline CDComDoc* CDComView::GetDocument() const
   { return reinterpret_cast<CDComDoc*>(m_pDocument); }
#endif

