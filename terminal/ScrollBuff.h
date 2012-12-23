#pragma once
#include <deque>
#include <vector>
#include <list>
#include <Windows.h>
#include <assert.h>

#undef  CATCH_DEBUG

#define SB_NUM_COLS         180
#define SB_MIN_COLS         80
#define SB_ANSI_NUM_LINES   25

#define SCROLLBUFF_DEFAULT_CR       0xffffffff
#define SCROLLBUFF_BLACK_CR         RGB(0x00, 0x00, 0x00)
#define SCROLLBUFF_RED_CR           RGB(0xc0, 0x00, 0x00)
#define SCROLLBUFF_GREEN_CR         RGB(0x00, 0xc0, 0x00)
#define SCROLLBUFF_YELLOW_CR        RGB(0xc0, 0xc0, 0x00)
#define SCROLLBUFF_BLUE_CR          RGB(0x00, 0x00, 0xc0)
#define SCROLLBUFF_MAGENTA_CR       RGB(0xc0, 0x00, 0xc0)
#define SCROLLBUFF_CYAN_CR          RGB(0x00, 0xc0, 0xc0)
#define SCROLLBUFF_WHITE_CR         RGB(0xc0, 0xc0, 0xc0)
#define SCROLLBUFF_MAKE_BOLD(cr)    ((cr == 0) ? RGB(0x80, 0x80, 0x80) : (RGB(GetRValue(cr) ? 0xff : 0x00, GetGValue(cr) ? 0xff : 0x00, GetBValue(cr) ? 0xff : 0x00)))


#define SCROLLBUFF_CFG_EMM_EMMULATE 0
#define SCROLLBUFF_CFG_EMM_SKIP     1
#define SCROLLBUFF_CFG_EMM_PASS     2

static inline char *mystrnchr(char *pText, int nTextLen, char chSearch)
{
    int nIndex;
    for (nIndex = 0; nIndex < nTextLen; nIndex++)
    {
        if (pText[nIndex] == chSearch)
        {
            return &(pText[nIndex]);
        }
    }
    return NULL;
}

static inline char *mystrnstr(char *pText, int nTextLen, char *szSearchStr)
{
    char *pRet = NULL;
    char *pTmp;
    int nIndex = 0;
    if (pText)
    {
        while (*szSearchStr)
        {
            pTmp = mystrnchr(pText, nTextLen, *szSearchStr);
            if (pTmp)
            {
                if ((pRet == NULL) || (pRet > pTmp))
                {
                    pRet = pTmp;
                }
            }
            szSearchStr++;
        }
    }
    return pRet;
}

static inline bool isbackspace(char *pText, int nTextLen)
{
    if ((nTextLen >= 3) && (memcmp(pText, "\010 \010", 3) == 0))
    {
        return true;
    }
    return false;
}


static inline bool myisalpha(char ch)
{
    if (((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z')))
    {
        return true;
    }
    else
    {
        return false;
    }
}

static inline bool myisnum(char ch)
{
    if ((ch >= '0') && (ch <= '9'))
    {
        return true;
    }
    else
    {
        return false;
    }
}

static inline char *findalpha(char* pText, int nTextLen)
{
    int nIndex = 0;
    while (pText && (nIndex < nTextLen))
    {
        if (myisalpha(pText[nIndex]) || ((pText[nIndex]) == '@'))
        {
            return &pText[nIndex];
        }
        nIndex++;
    }
    return NULL;
}

static inline bool myisprint(char ch)
{
    if (((ch >= 0x20) && (ch <= 0x7e)) || (ch == 0x09) || (ch == 0x0a) || (ch == 0x0d))
    {
        return true;
    }
    return false;
}

static inline char *findchartype(char *pText, int nTextLen, bool bPrintable)
{
    int nIndex = 0;
    while (pText && (nIndex < nTextLen))
    {
        if (myisprint(pText[nIndex]) == bPrintable)
        {
            return &(pText[nIndex]);
        }
        nIndex++;
    }
    return NULL;
}

static inline int StrnCpy(char *szDest, const char *szSrc, int nMax)
{
    int nRet = 0;
    while ((nRet < (nMax - 1)) && (szSrc[nRet]))
    {
        szDest[nRet] = szSrc[nRet];
        nRet++;
    }
    szDest[nRet] = 0;
    return nRet;
}

static inline int Vec2Char(char *szDest, std::list<char> *szSrc, int nMax)
{
    int nRet = 0;
    std::list<char>::iterator vPos;
    if (szSrc->size() > 0)
    {
        for (vPos = szSrc->begin(); (nRet < (nMax - 1)) && (vPos != szSrc->end()); nRet++, vPos++)
        {
            szDest[nRet] = *vPos;
        }
        szDest[nRet] = 0;
        if (nRet > 0)
        {
            szSrc->erase(szSrc->begin(), vPos);
        }
    }
    return nRet;
}

enum ETextType
{
    LINEBUF_BEEP = '\007',
    LINEBUF_NEWLINE = '\n',
    LINEBUF_BACKSPACE = '\010',
    LINEBUF_TAB = '\t',
    LINEBUF_ESCAPE_BRACKET = '\033',
    LINEBUF_CARRIAGE_RETURN = '\r',
    LINEBUF_TEXT = 256,
    LINEBUF_BROKEN_ESCAPE = 257,
    LINEBUF_ESCAPE_BARE = 258,
    LINEBUF_NON_PRINTABLE = 259,
};


#define SB_ATTR_NEGITAVE    0x00000001
#define SB_ATTR_BLINK       0x00000002
#define SB_ATTR_BOLD        0x00000004

struct STextRendition
{
    UINT32 nAttributes;
    COLORREF crBack;
    COLORREF crText;

    bool operator!=(const STextRendition &other) const
    {
        if ((nAttributes != other.nAttributes) ||
            (crBack != other.crBack) ||
            (crText != other.crText))
        {
            return true;
        }
        else
        {
            return false;
        }
    };
};

struct SFullLine
{
    STextRendition sTextRendition[SB_NUM_COLS+1];
    char szLine[SB_NUM_COLS+1];
};

struct SSelection
{
    long x;
    long y;
};

class CScrollBufL3
{
public:
    CScrollBufL3();
    std::deque<SFullLine> m_vecLine;
    std::vector<SFullLine> m_vecDeletedLine;

    virtual void SetMaxNumLines(long nMaxNumLines);
    virtual long GetMaxNumLines() { return m_nMaxNumLines; };

    virtual long GetNumLines();

    virtual void ClearAllLines();
    virtual void ClearLine(int y);

    virtual STextRendition GetDefaultRendition();
    virtual bool UpdateRendition(int x, int y, int nLen, STextRendition *sTextRendition);

    virtual long AllocateLines(long nInsertionLine);
protected:
    virtual long RemoveLines();

    long m_nMaxNumLines;
};

class CScrollBufL2
{
public:
    CScrollBufL2();
    ~CScrollBufL2();

    virtual void SetMaxNumLines(long nMaxNumLines) { m_cbScreen.SetMaxNumLines(nMaxNumLines); };
    virtual long GetNumLines() { return m_cbScreen.GetNumLines(); };

    virtual int  GetLineWidth() { return m_nMaxCols; };
    virtual void SetLineWidth(int nMaxCols);
    virtual long GetTop();
    virtual CScrollBufL3 *GetScreen(long *nLine);

    virtual long GetInsertionLine();
    virtual void SetInsertionLine(long nLine, bool bNewLine);
    virtual int  GetInsertionCol();
    virtual void SetInsertionCol(int nCol);
    virtual void KeypadInsertionCol(int nNumCols, int nSign);

    virtual bool ProcessScrollArea(int nStart, int nEnd);
    virtual bool ProcessDeleteChar(int nNumChars);
    virtual bool ProcessInsertChar(int nNumChars);

    virtual SFullLine *GetLine(long nLine, bool *bValid);
    virtual int SetLine(char *szText, int nLen, STextRendition *sTextRendition);
    
    virtual char GetChar(long nLine, int nCol);
    virtual int SetChar(long nLine, int nCol, char szText, int nLen, STextRendition *sTextRendition);

    virtual void SetWrap(bool bWrap) { m_bWrap = bWrap; };

    virtual void ClearAllLines();

    virtual void SaveCoords();
    virtual void RestoreCoords();

    virtual void InsertNewline();
    virtual std::vector<SFullLine> *GetDeleted(bool bSubScreen) 
    { 
        return bSubScreen                     ? 
            &(m_cbSubScreen.m_vecDeletedLine) :
            &(m_cbScreen.m_vecDeletedLine);
    }

protected:
    virtual void BoundsCheckCol(int *nCol, bool bWrap);
    virtual void AllocateLines(int nInsertionLine, CScrollBufL3 *pL3);

    int m_nInsertionCol;
    long m_nInsertionLine;

    int m_nSaveInsertionCol;
    long m_nSaveInsertionLine;

    long m_nStartScrollLine;
    long m_nEndScrollLine;
    int m_nMaxCols;

    bool m_bWrap;

    CScrollBufL3 m_cbScreen;
    CScrollBufL3 m_cbSubScreen;

    SFullLine m_sGetLineRet;
};

class CScrollBufL1
{
public:
    CScrollBufL1();
    ~CScrollBufL1();

    virtual bool FindText(CString szFindString, BOOL bFindMatchCase, BOOL bFindSearchDown, SSelection *sStartingLine);
    virtual long AddText(char *szText, int nBytes);
    virtual SFullLine *GetLine(long nLine, bool *bValid);
    virtual long GetNumLines() {return m_pL2->GetNumLines();};
    virtual void ClearAllLines();

    virtual int  GetCaretCol();
    virtual long GetCaretLine();

    virtual void ResetSelectionRange(SSelection sNewSelection, bool bDoubleClickDown);
    virtual void SetSelectionRange(SSelection sEndSelection);
    virtual void GetSelectedText(std::string *szText);
    virtual bool IsAnythingSelected();

    virtual void SetLineWidth(int nMaxCols);
    virtual void SetMaxNumLines(long nMaxNumLines);
    virtual void SetEmulation(int nEmulation);

    void CaptureStart(char *szFilename);
    void CaptureStop();
    void CaptureFlush();
    long CaptureDeleted(std::vector<SFullLine> *vecDeletedLine);

    virtual void Enter();
    virtual void Exit();
    virtual void VerifyCritical() 
    {
        ASSERT(m_bHaveCritical == true);
    };

protected:
    virtual void ParseText(char *szText, int nTextLen, ETextType *eTextType, int *nLineLen);
    virtual void ParseEscapeBracket(char *szText, int nTextLen, ETextType *eTextType, int *nLineLen);
    virtual void FindNextToken(char *szText, int nTextLen, ETextType *eTextType, int *nLineLen);
    virtual bool ExtractCodes(char *pText, int nLen, int *param1, int *param2, int *param3, char *command);
    virtual void ResetRendition();

    virtual long FillInData(char *szText, ETextType eTextType, int nLen);
    virtual bool UpdateText(char *szText, int nLen);

    virtual void ProcessMoveCursor(int nLine, int nCol);
    virtual bool ProcessResetMode(bool bExpanded, int param1);
    virtual bool ProcessMode(bool bExpanded, int param1);
    virtual bool ProcessEraseScreen(int param1, char cEraseChar);
    virtual bool ProcessEraseLine(int param1, long nLine, char cEraseChar);
    virtual bool ProcessEscape (char *szText, int nLen);
    virtual bool ProcessBarePound(char param);
    virtual bool ProcessBareEscape(char *szText, int nLen);
    virtual bool ProcessColor(int param);

    virtual void GetSelected(long nLine, long *nStartChar, long *nEndChar);
    virtual void SetSelected(SFullLine *sFullLine, long nStartChar, long nEndChar);
    virtual void OrderSelection(SSelection *sStartSelection, SSelection *sEndSelection);
    virtual void DoubleSelection(long nLine, long *nStartChar, long *nEndChar);
    virtual bool IsSelectionBackwards();
    virtual long UpdateSelection();

    virtual void AddDebug(char *szText, int nBytes);

    CScrollBufL2 *m_pL2;
    CScrollBufL2 m_l2Primary;
    CScrollBufL2 m_l2Secondary;

    int m_nBrokenEscLen;
    char m_szEscCode[32];
    char m_szSearchStr[10];

    int m_nEmulation;
    STextRendition m_sTextRendition;
    STextRendition m_sSaveTextRendition;

    bool m_bDoubleClickDown;
    long m_nDoubleSelectionLine;
    long m_nDoubleSelectionStart;
    long m_nDoubleSelectionEnd;
    SSelection m_sStartSelection;
    SSelection m_sEndSelection;

    CRITICAL_SECTION m_csVectorProtector;
    bool m_bHaveCritical;

    FILE *m_fCapture;
    long m_nCaptureStart;

#ifdef CATCH_DEBUG
    FILE *m_fDebugBin;
    FILE *m_fDebugTxt;
#endif
};

