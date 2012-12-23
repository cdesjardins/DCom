#include <stdafx.h>
#include <algorithm>
#include "scrollbuff.h"

template <typename CHAR_TYPE>
CHAR_TYPE *stristr
(
   CHAR_TYPE         *  szStringToBeSearched,
   const CHAR_TYPE   *  szSubstringToSearchFor
)
{
   CHAR_TYPE   *  pPos = NULL;
   CHAR_TYPE   *  szCopy1 = NULL;
   CHAR_TYPE   *  szCopy2 = NULL;

   // verify parameters
   if ((szStringToBeSearched == NULL) ||
        (szSubstringToSearchFor == NULL))
   {
      return szStringToBeSearched;
   }

   // empty substring - return input (consistent with strstr)
   if (_tcslen(szSubstringToSearchFor) == 0) 
   {
      return szStringToBeSearched;
   }

   szCopy1 = _tcslwr(_tcsdup(szStringToBeSearched));
   szCopy2 = _tcslwr(_tcsdup(szSubstringToSearchFor));

   if ((szCopy1 == NULL) || (szCopy2 == NULL))
   {
      // another option is to raise an exception here
      free((void*)szCopy1);
      free((void*)szCopy2);
      return NULL;
   }

   pPos = strstr(szCopy1, szCopy2);

   if ( pPos != NULL ) 
   {
      // map to the original string
      pPos = szStringToBeSearched + (pPos - szCopy1);
   }

   free((void*)szCopy1);
   free((void*)szCopy2);

   return pPos;
} // stristr(...)

CScrollBufL1::CScrollBufL1()
{
    m_pL2 = &m_l2Primary;
    InitializeCriticalSectionAndSpinCount(&m_csVectorProtector, 0x1000);
    sprintf(m_szSearchStr, "%c%c%c%c%c",
        LINEBUF_CARRIAGE_RETURN,
        LINEBUF_NEWLINE,
        LINEBUF_TAB,
        LINEBUF_BACKSPACE,
        LINEBUF_ESCAPE_BRACKET);
#ifdef CATCH_DEBUG
    m_fDebugBin = NULL;
    m_fDebugTxt = NULL;
#endif

    m_fCapture = NULL;
    m_nCaptureStart = 0;

    m_nBrokenEscLen = 0;
    m_bHaveCritical = false;
    m_nEmulation = SCROLLBUFF_CFG_EMM_EMMULATE;
    ClearAllLines();
}

CScrollBufL1::~CScrollBufL1()
{
    CaptureStop();
    DeleteCriticalSection(&m_csVectorProtector);

#ifdef CATCH_DEBUG
    if (m_fDebugBin != NULL)
    {
        fclose(m_fDebugBin);
        m_fDebugBin = NULL;
    }
    if (m_fDebugTxt != NULL)
    {
        fclose(m_fDebugTxt);
        m_fDebugTxt = NULL;
    }
#endif
}

void CScrollBufL1::SetEmulation(int nEmulation)
{
    m_nEmulation = nEmulation;
}

void CScrollBufL1::Enter()
{
    EnterCriticalSection(&m_csVectorProtector);
    m_bHaveCritical = true;
}

void CScrollBufL1::Exit()
{
    m_bHaveCritical = false;
    LeaveCriticalSection(&m_csVectorProtector);
}


void CScrollBufL1::SetMaxNumLines(long nMaxNumLines)
{
    long nTrueMax;

    if (nMaxNumLines < (SB_ANSI_NUM_LINES * 10))
    {
        nTrueMax = (SB_ANSI_NUM_LINES * 10);
    }
    else
    {
        nTrueMax = nMaxNumLines;
    }
    m_l2Primary.SetMaxNumLines(nTrueMax);
    m_l2Secondary.SetMaxNumLines(nTrueMax);
}

void CScrollBufL1::SetLineWidth(int nMaxCols)
{
    m_l2Primary.SetLineWidth(nMaxCols);
    m_l2Secondary.SetLineWidth(nMaxCols);
}

void CScrollBufL1::ProcessMoveCursor(int nLine, int nCol)
{
    VerifyCritical();
    if (nLine > -1)
    {
        nLine -= 1;
        if (nLine < 0)
        {
            nLine = 0;
        }
        m_pL2->SetInsertionLine(m_pL2->GetTop() + nLine, false);
    }
    m_pL2->SetInsertionCol((nCol == 0) ? 0 : nCol - 1);
}

bool CScrollBufL1::ProcessColor(int param)
{
    VerifyCritical();
    switch (param)
    {
    case 0 : /* reset */
        ResetRendition();
        break;
    case 1 : /* bold */
        m_sTextRendition.nAttributes |= SB_ATTR_BOLD;
        break;
    case 2 : /* faint */
        m_sTextRendition.nAttributes &= ~SB_ATTR_BOLD;
        break;
    case 3 : /* italic */
        TRACE("Color=italic\n");
        break;
    case 4 : /* single underline */
        TRACE("Color=single underline\n");
        break;
    case 5 : /* slow blink */
        m_sTextRendition.nAttributes |= SB_ATTR_BLINK;
        break;
    case 6 : /* rapid blink */
        TRACE("Color=rapid blink\n");
        break;
    case 7 : /* negative */
        m_sTextRendition.nAttributes |= SB_ATTR_NEGITAVE;
        break;
    case 8 : /* conceal */
        TRACE("Color=conceal\n");
        break;
    case 21: /* double underline */
        TRACE("Color=double underline\n");
        break;
    case 22: /* normal intensity */
        TRACE("Color=normal intensity\n");
        break;
    case 24: /* no underline */
        TRACE("Color=no underline\n");
        break;
    case 25: /* blink off */
        m_sTextRendition.nAttributes &= ~SB_ATTR_BLINK;
        break;
    case 27: /* positive */
        m_sTextRendition.nAttributes &= ~SB_ATTR_NEGITAVE;
        break;
    case 28: /* reveal */
        TRACE("Color=reveal\n");
        break;
    case 30: /* black */
        m_sTextRendition.crText = SCROLLBUFF_BLACK_CR;
        break;
    case 40:
        m_sTextRendition.crBack = SCROLLBUFF_BLACK_CR;
        break;
    case 31: /* red */
        m_sTextRendition.crText = SCROLLBUFF_RED_CR;
        break;
    case 41:
        m_sTextRendition.crBack = SCROLLBUFF_RED_CR;
        break;
    case 32: /* green */
        m_sTextRendition.crText = SCROLLBUFF_GREEN_CR;
        break;
    case 42:
        m_sTextRendition.crBack = SCROLLBUFF_GREEN_CR;
        break;
    case 33: /* yellow */
        m_sTextRendition.crText = SCROLLBUFF_YELLOW_CR;
        break;
    case 43:
        m_sTextRendition.crBack = SCROLLBUFF_YELLOW_CR;
        break;
    case 34: /* blue */
        m_sTextRendition.crText = SCROLLBUFF_BLUE_CR;
        break;
    case 44:
        m_sTextRendition.crBack = SCROLLBUFF_BLUE_CR;
        break;
    case 35: /* magenta */
        m_sTextRendition.crText = SCROLLBUFF_MAGENTA_CR;
        break;
    case 45:
        m_sTextRendition.crBack = SCROLLBUFF_MAGENTA_CR;
        break;
    case 36: /* cyan */
        m_sTextRendition.crText = SCROLLBUFF_CYAN_CR;
        break;
    case 46:
        m_sTextRendition.crBack = SCROLLBUFF_CYAN_CR;
        break;
    case 37: /* white */
        m_sTextRendition.crText = SCROLLBUFF_WHITE_CR;
        break;
    case 47:
        m_sTextRendition.crBack = SCROLLBUFF_WHITE_CR;
        break;
    case 39: /* reset color */
        m_sTextRendition.crText = SCROLLBUFF_DEFAULT_CR;
        break;
    case 49:
        m_sTextRendition.crBack = SCROLLBUFF_DEFAULT_CR;
        break;
    }
    return true;
}

bool CScrollBufL1::ProcessMode(bool bExpanded, int param1)
{
    VerifyCritical();
    if (bExpanded)
    {
        switch (param1)
        {
        case 1:
            //TRACE("Cursor\n");
            break;
        case 3:
            ProcessEraseScreen(2, ' ');
            m_pL2->SetLineWidth(132);
            break;
        case 5: /* Reverse video */
            TRACE("Reverse video\n");
            break;
        case 7: /* Wrap around */
            m_pL2->SetWrap(true);
            break;
        case 25:
            //TRACE("Show cursor\n");
            break;
        case 1049:
            m_pL2 = &m_l2Secondary;
            break;
        default:
            TRACE("ProcessMode Expanded %i\n", param1);
            break;
        }
    }
    else
    {
        TRACE("ProcessMode %i\n", param1);
    }
    return true;
}

bool CScrollBufL1::ProcessResetMode(bool bExpanded, int param1)
{
    VerifyCritical();
    if (bExpanded)
    {
        switch (param1)
        {
        case 1:
            //TRACE("Reset cursor\n");
            break;
        case 3:
            ProcessEraseScreen(2, ' ');
            m_pL2->SetLineWidth(80);
            break;
        case 5: /* Normal video */
            TRACE("Normal video\n");
            break;
        case 7: /* Wrap around */
            m_pL2->SetWrap(false);
            break;
        case 25:
            //TRACE("Hide cursor\n");
            break;
        case 1049:
            m_pL2 = &m_l2Primary;
            break;
        default:
            TRACE("ProcessResetMode Expanded %i\n", param1);
            break;
        }        
    }
    else
    {
        TRACE("ProcessResetMode %i\n", param1);
    }
    return true;
}

bool CScrollBufL1::ProcessEraseScreen(int param1, char cEraseChar)
{
    int nCnt, nEnd;
    VerifyCritical();
    CaptureFlush();
    switch (param1)
    {
    case -1: /* Erase to end of screen */
    case 0:
        ProcessEraseLine(0, m_pL2->GetInsertionLine(), cEraseChar);
        for (nCnt = m_pL2->GetInsertionLine(); nCnt < m_pL2->GetNumLines(); nCnt++)
        {
            ProcessEraseLine(2, nCnt, cEraseChar);
        }
        break;
    case 1: /* Erase to beginning of screen */
        ProcessEraseLine(1, m_pL2->GetInsertionLine(), cEraseChar);
        for (nCnt = m_pL2->GetTop(); nCnt < m_pL2->GetInsertionLine(); nCnt++)
        {
            ProcessEraseLine(2, nCnt, cEraseChar);
        }
        break;
    case 2: /* Erase screen */
        nEnd = m_pL2->GetTop() + SB_ANSI_NUM_LINES;
        for (nCnt = m_pL2->GetTop(); nCnt < nEnd; nCnt++)
        {
            ProcessEraseLine(2, nCnt, cEraseChar);
        }
        ProcessMoveCursor(0, 0);
        break;
    }
    return true;
}

bool CScrollBufL1::ProcessEraseLine(int param1, long nLine, char cEraseChar)
{
    int nStart, nEnd;
    VerifyCritical();

    switch (param1)
    {
    case -1: /* Erase to eol */
    case 0:
        nStart = m_pL2->GetInsertionCol();
        nEnd = m_pL2->GetLineWidth();
        break;
    case 1: /* Erase to bol */
        nStart = 0;
        nEnd = m_pL2->GetInsertionCol() + 1;
        break;
    case 2: /* Erase line */
        nStart = 0;
        nEnd = m_pL2->GetLineWidth();
        break;
    }
    m_pL2->SetChar(nLine, nStart, cEraseChar, nEnd - nStart, (cEraseChar == ' ') ? &m_sTextRendition : NULL);
    return true;
}

bool CScrollBufL1::ExtractCodes(char *pText, int nLen, int *param1, int *param2, int *param3, char *command)
{
    bool bExpanded = false;
    char *p;
    VerifyCritical();

    pText = strchr(pText, '[');
    if (pText)
    {
        pText++;
        p = findalpha(pText, nLen);
        if (p)
        {
            *command = *p;
        }
        p = strchr(pText, ';');
        if ((p) && (((int)p - (int)pText) <= nLen))
        {
            *param2 = atoi(p + 1);
            p = strchr(p + 1, ';');
            if ((p) && (((int)p - (int)pText) <= nLen))
            {
                *param3 = atoi(p + 1);
            }
        }
        if (*pText == '?')
        {
            pText++;
            bExpanded = true;
        }
        if (myisnum(*pText))
        {
            *param1 = atoi(pText);
        }
    }
    return bExpanded;
}

bool CScrollBufL1::ProcessBarePound(char param)
{
    VerifyCritical();
    switch (param)
    {
    case '8':
        ProcessEraseScreen(2, 'E');
        break;
    }
    return true;
}

bool CScrollBufL1::ProcessBareEscape(char *szText, int nLen)
{
    VerifyCritical();
    switch (szText[1])
    {
    case 'D':
        m_pL2->SetInsertionLine(m_pL2->GetInsertionLine() + 1, false);
        break;
    case 'E':
        m_pL2->InsertNewline();
        break;
    case 'M':
        m_pL2->SetInsertionLine(m_pL2->GetInsertionLine() - 1, false);
        break;
    case '7':
        m_pL2->SaveCoords();
        m_sSaveTextRendition = m_sTextRendition;
        break;
    case '8':
        m_pL2->RestoreCoords();
        m_sTextRendition = m_sSaveTextRendition;
        break;
    case '>':
        TRACE("Numeric keypad\n");
        break;
    case '=':
        TRACE("Application keypad\n");
        break;
    case '#':
        /* double width/height text */
        /* or screen alignment test */
        ProcessBarePound(szText[2]);
        break;
    case ']':
        /* ESC ] 0 ;<label>^G	 - Set window and icon labels to <label> */
        break;
    default:
        TRACE("ProcessBareEscape %c\n", szText[1]);
        break;
    }
    return true;
}

bool CScrollBufL1::ProcessEscape(char *szText, int nLen)
{
    int param1 = -1;
    int param2 = -1;
    int param3 = -1;
    char command = 0;
    bool bExpanded;

    VerifyCritical();
    bExpanded = ExtractCodes(szText, nLen, &param1, &param2, &param3, &command);

    switch (command)
    {
    case 'c':
        /* Terminal ID aka device attribute */
        TRACE("Terminal ID\n");
        break;
    case 'A':
        m_pL2->SetInsertionLine(m_pL2->GetInsertionLine() - param1, false);
        break;
    case 'B':
        m_pL2->SetInsertionLine(m_pL2->GetInsertionLine() + param1, false);
        break;
    case 'C':
        m_pL2->KeypadInsertionCol(param1, 1);
        break;
    case 'D':
        m_pL2->KeypadInsertionCol(param1, -1);
        break;
    case 'G':
        m_pL2->SetInsertionCol(param1);
        break;
    case 'h':
        ProcessMode(bExpanded, param1);
        break;
    case 'f':
    case 'H':
        ProcessMoveCursor(param1, param2);
        break;
    case 'J':
        ProcessEraseScreen(param1, ' ');
        break;
    case 'K':
        ProcessEraseLine(param1, m_pL2->GetInsertionLine(), ' ');
        break;
    case 'l':
        ProcessResetMode(bExpanded, param1);
        break;
    case 'm':
        if (param1 == -1)
        {
            ProcessColor(0);
        }
        else
        {
            ProcessColor(param1);
        }
        if (param2 != -1)
        {
            ProcessColor(param2);
        }
        if (param3 != -1)
        {
            ProcessColor(param3);
        }
        break;
    case 'P':
        m_pL2->ProcessDeleteChar(param1);
        break;
    case 'r':
        m_pL2->ProcessScrollArea(param1, param2);
        break;
    case '@':
        m_pL2->ProcessInsertChar(param1);
        break;
    default:
        TRACE3("Got %i %i %c\n", param1, param2, command);
        break;
    }

    return true;
}

bool CScrollBufL1::UpdateText(char *szText, int nLen)
{
    int nLenDone;
    VerifyCritical();
    do
    {
        nLenDone = m_pL2->SetLine(szText, nLen, &m_sTextRendition);
        szText += nLenDone;
        nLen -= nLenDone;
    } while (nLen > 0);
    return true;
}

bool CScrollBufL1::IsSelectionBackwards()
{
    if ((m_sStartSelection.y > m_sEndSelection.y) || 
        ((m_sStartSelection.y == m_sEndSelection.y) && 
        (m_sStartSelection.x > m_sEndSelection.x)))
    {
        return true;
    }
    return false;
}

void CScrollBufL1::OrderSelection(SSelection *sStartSelection, SSelection *sEndSelection)
{
    *sStartSelection = m_sStartSelection;
    *sEndSelection = m_sEndSelection;
    if (IsSelectionBackwards())
    {
        *sStartSelection = m_sEndSelection;
        *sEndSelection = m_sStartSelection;
    }
}

void MoveSelectionDown(char *pText, long *nCharIndex, int isfn(int))
{
    while (((*nCharIndex) > 0) && (isfn(pText[(*nCharIndex) - 1])))
    {
        (*nCharIndex)--;
    }
}
void MoveSelectionUp(char *pText, long *nCharIndex, int isfn(int))
{
    while (((*nCharIndex) < SB_NUM_COLS) && (isfn(pText[(*nCharIndex)])))
    {
        (*nCharIndex)++;
    }
}

int isnotspace(int c)
{
    return !isspace(c);
}

void CScrollBufL1::DoubleSelection(long nLine, long *nStartChar, long *nEndChar)
{
    char *pText;
    CScrollBufL3 *pL3;

    pL3 = m_pL2->GetScreen(&nLine);
    if (nLine < (long)pL3->GetNumLines())
    {
        pText = pL3->m_vecLine[nLine].szLine;
        if ((*nStartChar) == (*nEndChar))
        {
            if (isspace(pText[*nStartChar]))
            {
                MoveSelectionDown(pText, nStartChar, isspace);
                MoveSelectionUp(pText, nEndChar, isspace);
            }
            else
            {
                MoveSelectionDown(pText, nStartChar, isnotspace);
                MoveSelectionUp(pText, nEndChar, isnotspace);
            }
            m_nDoubleSelectionStart = (*nStartChar);
            m_nDoubleSelectionEnd = (*nEndChar);
            m_nDoubleSelectionLine = nLine;
        }
        else
        {
            if ((m_bDoubleClickDown) && 
                (m_nDoubleSelectionStart != m_nDoubleSelectionEnd) && 
                (nLine == m_nDoubleSelectionLine))
            {
                if (IsSelectionBackwards())
                {
                    (*nEndChar) = m_nDoubleSelectionEnd;
                }
                else
                {
                    (*nStartChar) = m_nDoubleSelectionStart;
                }
            }

            if (IsSelectionBackwards())
            {
                if (isspace(pText[*nStartChar]))
                {
                    MoveSelectionDown(pText, nStartChar, isspace);
                }
                else
                {
                    MoveSelectionDown(pText, nStartChar, isnotspace);
                }
            }
            else
            {
                if (isspace(pText[*nEndChar]))
                {
                    MoveSelectionUp(pText, nEndChar, isspace);
                }
                else
                {
                    MoveSelectionUp(pText, nEndChar, isnotspace);
                }
            }
        }
    }
}

void CScrollBufL1::GetSelected(long nLine, long *nStartChar, long *nEndChar)
{
    SSelection sStartSelection;
    SSelection sEndSelection;
    VerifyCritical();
    
    OrderSelection(&sStartSelection, &sEndSelection);
    *nStartChar = -1;
    *nEndChar = -1;
    if ((nLine > sStartSelection.y) && (nLine < sEndSelection.y))
    {
        *nStartChar = 0;
        *nEndChar = SB_NUM_COLS - 1;
    }
    else if ((nLine == sStartSelection.y) && (nLine == sEndSelection.y))
    {
        *nStartChar = sStartSelection.x;
        *nEndChar = sEndSelection.x;
    }
    else if ((nLine == sStartSelection.y) && (nLine < sEndSelection.y))
    {
        *nStartChar = sStartSelection.x;
        *nEndChar = SB_NUM_COLS - 1;
    }
    else if ((nLine > sStartSelection.y) && (nLine == sEndSelection.y))
    {
        *nStartChar = 0;
        *nEndChar = sEndSelection.x;
    }
    if (*nStartChar >= SB_NUM_COLS)
    {
        *nStartChar = SB_NUM_COLS;
    }

    if (*nEndChar >= SB_NUM_COLS)
    {
        *nEndChar = SB_NUM_COLS;
    }
    if (!(((*nStartChar) == -1) && ((*nEndChar) == -1)))
    {
        if (m_bDoubleClickDown)
        {
            DoubleSelection(nLine, nStartChar, nEndChar);
        }
    }
}

void CScrollBufL1::SetSelected(SFullLine *sFullLine, long nStartChar, long nEndChar)
{
    long nIndex;
    VerifyCritical();
    for (nIndex = nStartChar; nIndex < nEndChar; nIndex++)
    {
        sFullLine->sTextRendition[nIndex].nAttributes ^= SB_ATTR_NEGITAVE;
    }
}

void CScrollBufL1::ResetSelectionRange(SSelection sNewSelection, bool bDoubleClickDown)
{
    Enter();
    m_sStartSelection = sNewSelection;
    m_sEndSelection = sNewSelection;
    m_bDoubleClickDown = bDoubleClickDown;
    Exit();
}

void CScrollBufL1::SetSelectionRange(SSelection sEndSelection)
{
    Enter();
    m_sEndSelection = sEndSelection;
    Exit();
}

bool CScrollBufL1::IsAnythingSelected()
{
    if ((m_sStartSelection.x == m_sEndSelection.x) &&
        (m_sStartSelection.y == m_sEndSelection.y))
    {
        return false;
    }
    return true;
}

void CScrollBufL1::GetSelectedText(std::string *szText)
{
    CScrollBufL3 *pL3;
    SSelection sStartSelection;
    SSelection sEndSelection;
    long nIndex;
    long nLine;
    long nLen;
    long nStartChar, nEndChar;
    VerifyCritical();
    
    OrderSelection(&sStartSelection, &sEndSelection);
    
    szText->clear();
    for (nIndex = sStartSelection.y; nIndex <= sEndSelection.y; nIndex++)
    {
        nLine = nIndex;
        GetSelected(nLine, &nStartChar, &nEndChar);
        pL3 = m_pL2->GetScreen(&nLine);
        nLen = szText->size();
        if ((nLine < pL3->GetNumLines()) && (nStartChar <= (long)strlen(pL3->m_vecLine[nLine].szLine)))
        {
            szText->append(pL3->m_vecLine[nLine].szLine, nStartChar, (nEndChar - nStartChar));
            if (nEndChar == (SB_NUM_COLS - 1))
            {
                szText->append("\r\n");
            }
        }
    }
}

SFullLine *CScrollBufL1::GetLine(long nLine, bool *bValid)
{
    long nStartChar, nEndChar;
    SFullLine *pFullLine;
    VerifyCritical();

    GetSelected(nLine, &nStartChar, &nEndChar);
    
    pFullLine = m_pL2->GetLine(nLine, bValid);
    if (bValid)
    {
        SetSelected(pFullLine, nStartChar, nEndChar);
    }

    return pFullLine;
}

void CScrollBufL1::ResetRendition()
{
    CScrollBufL3 *pL3;
    long nLine = 0;
    VerifyCritical();
    pL3 = m_pL2->GetScreen(&nLine);
    m_sTextRendition = pL3->GetDefaultRendition();
}

void CScrollBufL1::ClearAllLines()
{
    Enter();
    CaptureFlush();
    m_l2Primary.ClearAllLines();
    m_l2Secondary.ClearAllLines();
    m_nCaptureStart = 0;
    m_sStartSelection.x = 0;
    m_sStartSelection.y = 0;
    m_sEndSelection.x = 0;
    m_sEndSelection.y = 0;
    m_bDoubleClickDown = false;
    m_nDoubleSelectionStart = 0;
    m_nDoubleSelectionEnd = 0;
    ResetRendition();
    Exit();
}

long CScrollBufL1::FillInData(char *szText, ETextType eTextType, int nLen)
{
    int nInsertionCol;
    char szTab[] = "                              ";
    Enter();
    if (m_nBrokenEscLen > 0)
    {
        szText = m_szEscCode;
    }
    switch (eTextType)
    {
    case LINEBUF_BEEP:
        MessageBeep(-1);
        break;
    case LINEBUF_TAB:
        nInsertionCol = (m_pL2->GetInsertionCol() & 7);
        nInsertionCol = ((nInsertionCol + 8) & (~7)) - nInsertionCol;
        UpdateText(szTab, nInsertionCol);
        break;
    case LINEBUF_BACKSPACE:
        m_pL2->SetInsertionCol(m_pL2->GetInsertionCol() - 1);
        break;
    case LINEBUF_CARRIAGE_RETURN:
        m_pL2->SetInsertionCol(0);
        break;
    case LINEBUF_NEWLINE:
        m_pL2->InsertNewline();
        break;
    case LINEBUF_TEXT:
        UpdateText(szText, nLen);
        break;
    case LINEBUF_ESCAPE_BRACKET:
        if (m_nEmulation == SCROLLBUFF_CFG_EMM_EMMULATE)
        {
            ProcessEscape(szText, nLen);
        }
        break;
    case LINEBUF_ESCAPE_BARE:
        if (m_nEmulation == SCROLLBUFF_CFG_EMM_EMMULATE)
        {
            ProcessBareEscape(szText, nLen);
        }
        break;
    }
    Exit();
    return 0;
}

void CScrollBufL1::FindNextToken(char *szText, int nTextLen, ETextType *eTextType, int *nLineLen)
{
    char *pTargetChar;
    bool bPrintable = myisprint(*szText);
    (*eTextType) = bPrintable ? LINEBUF_TEXT : LINEBUF_NON_PRINTABLE;

    pTargetChar = findchartype(szText, nTextLen, !bPrintable);
    if (pTargetChar)
    {
        (*nLineLen) = (int)pTargetChar - (int)szText;
    }
    else
    {
        (*nLineLen) = nTextLen;
    }

    pTargetChar = mystrnstr(szText, (*nLineLen), m_szSearchStr);
    if (pTargetChar)
    {
        (*nLineLen) = (int)pTargetChar - (int)szText;
    }
}

void CScrollBufL1::ParseEscapeBracket(char *szText, int nTextLen, ETextType *eTextType, int *nLineLen)
{
    char *pTargetChar;

    pTargetChar = mystrnstr(szText + 1, nTextLen - 1, "[DME78HgcAB0<>=\033");
    if (pTargetChar)
    {
        if (*pTargetChar == '\033')
        {
            TRACE("******* POSSIBLE PARSE PROBLEM %s\n", szText + 1);
            (*eTextType) = LINEBUF_NON_PRINTABLE;
        }
        else if (*pTargetChar == '[')
        {
            (*eTextType) = LINEBUF_ESCAPE_BRACKET;
            pTargetChar = findalpha(szText, nTextLen);
        }
        else
        {
            (*eTextType) = LINEBUF_ESCAPE_BARE;
        }
    }
    if (pTargetChar)
    {
        (*nLineLen) = ((int)pTargetChar - (int)szText) + 1;
    }
    else
    {
        (*eTextType) = LINEBUF_BROKEN_ESCAPE;
        memset(m_szEscCode, 0, sizeof(m_szEscCode));
        strncpy(m_szEscCode, szText, sizeof(m_szEscCode) - 1);
        (*nLineLen) = nTextLen;
        m_nBrokenEscLen = nTextLen;
    }
}

void CScrollBufL1::ParseText(char *szText, int nTextLen, ETextType *eTextType, int *nLineLen)
{
    if ((m_nEmulation == SCROLLBUFF_CFG_EMM_EMMULATE) || (m_nEmulation == SCROLLBUFF_CFG_EMM_SKIP))
    {
        if (m_nBrokenEscLen > 0)
        {
            strncat(m_szEscCode, szText, sizeof(m_szEscCode) - 1 - m_nBrokenEscLen);
            szText = m_szEscCode;
        }
        switch (*szText)
        {
        case LINEBUF_CARRIAGE_RETURN:
        case LINEBUF_BEEP:
        case LINEBUF_TAB:
        case LINEBUF_BACKSPACE:
        case LINEBUF_NEWLINE:
            (*eTextType) = (ETextType)(*szText);
            (*nLineLen) = 1;
            break;
        case LINEBUF_ESCAPE_BRACKET:
            ParseEscapeBracket(szText, nTextLen, eTextType, nLineLen);
            break;
        default:
            FindNextToken(szText, nTextLen, eTextType, nLineLen);
            break;
        }
    }
    else
    {
        FindNextToken(szText, nTextLen, eTextType, nLineLen);
    }
}

void CScrollBufL1::AddDebug(char *szText, int nBytes)
{
#ifdef CATCH_DEBUG
    if (m_fDebugBin == NULL)
    {
        static int ndbg = 0;
        char name[64];
        ndbg++;
        sprintf(name, "C:\\dl\\dbg%i.bin", ndbg);
        m_fDebugBin = fopen(name, "wb");
        sprintf(name, "C:\\dl\\dbg%i.txt", ndbg);
        m_fDebugTxt = fopen(name, "w");
    }
    if (m_fDebugBin != NULL)
    {
        fwrite(&nBytes, sizeof(nBytes), 1, m_fDebugBin);
        fwrite(szText, sizeof(char), nBytes, m_fDebugBin);
    }

    if (m_fDebugTxt != NULL)
    {
        fprintf(m_fDebugTxt, "%s", szText);
    }
#endif
}

long CScrollBufL1::UpdateSelection()
{
    long nRet;

    Enter();
    nRet = CaptureDeleted(m_pL2->GetDeleted(false));
    CaptureDeleted(m_pL2->GetDeleted(true));
    m_sStartSelection.y -= nRet;
    m_sEndSelection.y -= nRet;
    if (m_sStartSelection.y < 0)
    {
        m_sStartSelection.x = 0;
        m_sStartSelection.y = 0;
    }
    if (m_sEndSelection.y < 0)
    {
        m_sEndSelection.x = 0;
        m_sEndSelection.y = 0;
    }
    Exit();
    return nRet;
}

long CScrollBufL1::AddText(char *szText, int nBytes)
{
    int nLineLen;
    int nBytesProcessed = 0;
    ETextType eTextType;

    AddDebug(szText, nBytes);

    while (nBytesProcessed < nBytes)
    {
        ParseText(szText, nBytes - nBytesProcessed, &eTextType, &nLineLen);
        if (nLineLen > 0)
        {
            FillInData(szText, eTextType, nLineLen);
            if (eTextType != LINEBUF_BROKEN_ESCAPE)
            {
                szText += (nLineLen - m_nBrokenEscLen);
                nBytesProcessed += (nLineLen - m_nBrokenEscLen);
                m_nBrokenEscLen = 0;
            }
            else
            {
                szText += nLineLen;
                nBytesProcessed += nLineLen;
            }
        }
        else
        {
            szText++;
            nBytesProcessed++;
        }
    }
    
    return UpdateSelection();
}

long CScrollBufL1::GetCaretLine()
{
    long nRet;
    VerifyCritical();
    nRet = m_pL2->GetInsertionLine();
    return nRet;
}

int CScrollBufL1::GetCaretCol()
{
    long nRet;
    VerifyCritical();
    nRet = m_pL2->GetInsertionCol();
    return nRet;
}

bool CScrollBufL1::FindText(CString szFindString, BOOL bFindMatchCase, BOOL bFindSearchDown, SSelection *sStartingLine)
{
    long nEnd, nIncrement;
    bool bRet = false;
    SSelection sFound;
    bool bValid;
    SFullLine *sFullLine;
    char *pText;
    CScrollBufL3 *pL3;

    Enter();
    pL3 = m_pL2->GetScreen(&sStartingLine->y);
    if (bFindSearchDown)
    {
        nEnd = pL3->GetNumLines() - 1;
        nIncrement = 1;
        if ((sStartingLine->y > (long)pL3->GetNumLines()) || (sStartingLine->y < 0))
        {
            sStartingLine->y = 0;
        }
    }
    else
    {
        nEnd = 0;
        nIncrement = -1;
        if ((sStartingLine->y > (long)pL3->GetNumLines()) || (sStartingLine->y < 0))
        {
            sStartingLine->y = pL3->GetNumLines() - 1;
        }
    }
    
    while (sStartingLine->y != nEnd)
    {
        sFullLine = GetLine(sStartingLine->y, &bValid);
        if (bValid && sFullLine)
        {
            if (bFindMatchCase)
            {
                pText = strstr(sFullLine->szLine + sStartingLine->x, szFindString);
            }
            else
            {
                pText = stristr(sFullLine->szLine + sStartingLine->x, szFindString.GetBuffer());
            }
            if (pText)
            {
                sStartingLine->x = (pText - sFullLine->szLine) + szFindString.GetLength();
                sFound = *sStartingLine;
                ResetSelectionRange(sFound, false);
                sFound.x -= szFindString.GetLength();
                SetSelectionRange(sFound);
                bRet = true;
                break;
            }
        }
        sStartingLine->x = 0;
        sStartingLine->y += nIncrement;
    }
    Exit();
    return bRet;
}

void CScrollBufL1::CaptureStart(char *szFilename)
{
    m_fCapture = fopen(szFilename, "w");
    m_nCaptureStart = m_pL2->GetTop();
}

void CScrollBufL1::CaptureStop()
{

    if (m_fCapture)
    {
        Enter();
        CaptureFlush();
        Exit();
        fclose(m_fCapture);
        m_fCapture = NULL;
        m_nCaptureStart = 0;
    }
}

void CScrollBufL1::CaptureFlush()
{
    SFullLine *sFullLine;
    bool bValid;
    VerifyCritical();
    if (m_fCapture)
    {
        for (; m_nCaptureStart < GetNumLines(); m_nCaptureStart++)
        {
            sFullLine = GetLine(m_nCaptureStart, &bValid);
            if (bValid == true)
            {
                fprintf(m_fCapture, "%s\n", sFullLine->szLine);
            }
        }
    }
}

long CScrollBufL1::CaptureDeleted(std::vector<SFullLine> *vecDeletedLine)
{
    long nIndex;
    long nNumLinesDeleted = (long)vecDeletedLine->size();
    m_nCaptureStart -= nNumLinesDeleted;
    if (m_nCaptureStart < 0)
    {
        m_nCaptureStart = 0;
    }
    if ((m_fCapture) && (m_nCaptureStart == 0))
    {
        for (nIndex = 0; nIndex < nNumLinesDeleted; nIndex++)
        {
            fprintf(m_fCapture, "%s\n", (*vecDeletedLine)[nIndex].szLine);
        }
    }
    vecDeletedLine->clear();
    return nNumLinesDeleted;
}

/*********************************************************************
**
**  CScrollBufL2
**
*********************************************************************/

CScrollBufL2::CScrollBufL2()
{
    m_nMaxCols = SB_NUM_COLS - 1;
    m_nStartScrollLine = -1;
    m_nEndScrollLine = -1;
    m_bWrap = true;
}

CScrollBufL2::~CScrollBufL2()
{
}

void CScrollBufL2::ClearAllLines()
{
    m_nInsertionLine = 0;
    m_nInsertionCol = 0;
    m_cbScreen.ClearAllLines();
    m_cbSubScreen.ClearAllLines();
}

void CScrollBufL2::SetLineWidth(int nMaxCols)
{
    if (nMaxCols >= SB_NUM_COLS)
    {
        m_nMaxCols = SB_NUM_COLS - 1;
    }
    else if (nMaxCols < SB_MIN_COLS)
    {
        m_nMaxCols = SB_MIN_COLS;
    }
    else
    {
        m_nMaxCols = nMaxCols;
    }
    BoundsCheckCol(&m_nInsertionCol, false);
}

void CScrollBufL2::BoundsCheckCol(int *nCol, bool bWrap)
{
    if ((*nCol) < 0)
    {
        if (bWrap)
        {
            SetInsertionCol(m_nMaxCols - 1);
            SetInsertionLine(GetInsertionLine() - 1, true);
        }
        else
        {
            (*nCol) = 0;
        }
    }
    if ((*nCol) > m_nMaxCols)
    {
        if (bWrap)
        {
            InsertNewline();
        }
        else
        {
            (*nCol) = m_nMaxCols - 1;
        }
    }
}

CScrollBufL3 *CScrollBufL2::GetScreen(long *nLine)
{
    CScrollBufL3 *pRet;
    pRet = &m_cbScreen;
    if (((m_nStartScrollLine != -1) && 
        (m_nEndScrollLine != -1)) && 
        (m_nStartScrollLine != m_nEndScrollLine) &&
        (((*nLine) >= m_nStartScrollLine) && 
        ((*nLine) < m_nEndScrollLine)))
    {
        pRet = &m_cbSubScreen;
        (*nLine) -= m_nStartScrollLine;
    }
    AllocateLines(*nLine, pRet);
    return pRet;
}

void CScrollBufL2::SetInsertionLine(long nLine, bool bNewLine)
{
    long nDelta = 0;
    long nInsertionLine = 0;
    CScrollBufL3 *pL3;

    if (nLine < 0)
    {
        nLine = 0;
    }
    if (bNewLine)
    {
        nInsertionLine = m_nInsertionLine;
        nDelta = nLine - m_nInsertionLine;
    }
    else
    {
        if (nLine >= ((GetTop() + SB_ANSI_NUM_LINES) - 1))
        {
            nLine = GetTop() + SB_ANSI_NUM_LINES - 1;
        }
        nInsertionLine = nLine;
    }

    m_nInsertionLine = nLine;
    pL3 = GetScreen(&nInsertionLine);

    AllocateLines(nInsertionLine + nDelta, pL3);
}

void CScrollBufL2::SetInsertionCol(int nCol)
{
    long nInsertionLine;
    long nTmp;

    m_nInsertionCol = nCol;

    BoundsCheckCol(&m_nInsertionCol, m_bWrap);

    nInsertionLine = GetInsertionLine();
    for (nTmp = 0; nTmp < m_nInsertionCol; nTmp++)
    {
        if (GetChar(nInsertionLine, nTmp) == 0)
        {
            SetChar(nInsertionLine, nTmp, ' ', 1, NULL);
        }
    }
}

int CScrollBufL2::GetInsertionCol()
{
    return m_nInsertionCol;
}

void CScrollBufL2::KeypadInsertionCol(int nNumCols, int nSign)
{
    if (nNumCols == 0)
    {
        nNumCols = 1;
    }
    SetInsertionCol(GetInsertionCol() + (nNumCols * nSign));
}

long CScrollBufL2::GetTop()
{
    long ret;
    ret = m_cbScreen.GetNumLines() - SB_ANSI_NUM_LINES;
    if (ret < 0)
    {
        ret = 0;
    }
    return ret;
}

long CScrollBufL2::GetInsertionLine()
{
    return m_nInsertionLine;
}

SFullLine *CScrollBufL2::GetLine(long nLine, bool *bValid)
{
    CScrollBufL3 *pL3;
    pL3 = GetScreen(&nLine);
    (*bValid) = false;
    if (nLine < pL3->GetNumLines())
    {
        (*bValid) = true;
        memcpy(&m_sGetLineRet, &pL3->m_vecLine[nLine], sizeof(m_sGetLineRet));
    }
    else
    {
        memset(&m_sGetLineRet, 0, sizeof(m_sGetLineRet));
    }

    return &m_sGetLineRet;
}

int CScrollBufL2::SetLine(char *szText, int nLen, STextRendition *sTextRendition)
{
    CScrollBufL3 *pL3;
    long nLine;
    int nCol;

    nLine = GetInsertionLine();
    nCol = GetInsertionCol();

    if ((nLen + nCol) >= (m_nMaxCols))
    {
        nLen = m_nMaxCols - nCol;
    }
    if (nLen > 0)
    {
        pL3 = GetScreen(&nLine);
        if (sTextRendition)
        {
            pL3->UpdateRendition(nCol, nLine, nLen, sTextRendition);
        }
        memcpy(pL3->m_vecLine[nLine].szLine + nCol, szText, nLen);
        SetInsertionCol(GetInsertionCol() + nLen);
    }
    else
    {
        SetInsertionCol(GetInsertionCol() + 1);
    }
    return nLen;
}

int CScrollBufL2::SetChar(long nLine, int nCol, char szText, int nLen, STextRendition *sTextRendition)
{
    CScrollBufL3 *pL3;
    if (nLen > 0)
    {
        if ((nLen + nCol) >= (m_nMaxCols))
        {
            nLen = m_nMaxCols - nCol + 1;
        }
        pL3 = GetScreen(&nLine);
        if (sTextRendition)
        {
            pL3->UpdateRendition(nCol, nLine, nLen, sTextRendition);
        }
        memset(pL3->m_vecLine[nLine].szLine + nCol, szText, nLen);
    }
    return nLen;
}

char CScrollBufL2::GetChar(long nLine, int nCol)
{
    char chRet = 0;
    CScrollBufL3 *pL3;
    pL3 = GetScreen(&nLine);
    chRet = pL3->m_vecLine[nLine].szLine[nCol];
    return chRet;
}

bool CScrollBufL2::ProcessScrollArea(int nStart, int nEnd)
{
    if ((nStart != -1) && (nEnd != -1))
    {
        nStart--;
        if ((m_nStartScrollLine != nStart) || (m_nEndScrollLine != nEnd))
        {
            if (nStart == 0)
            {
                m_nStartScrollLine = -1;
                m_nEndScrollLine = -1;
            }
            else
            {
                m_nStartScrollLine = GetTop() + nStart;
                m_nEndScrollLine = GetTop() + nEnd;
                m_cbSubScreen.SetMaxNumLines(nEnd - nStart);
                AllocateLines(nEnd - nStart, &m_cbSubScreen);
            }
        }
    }
    else
    {
        m_nStartScrollLine = -1;
        m_nEndScrollLine = -1;
    }
    return true;
}

bool CScrollBufL2::ProcessDeleteChar(int nNumChars)
{
    long nInsertionLine = GetInsertionLine();
    int nStart, nEnd;
    CScrollBufL3 *pL3;
    STextRendition sTextRendition;

    pL3 = GetScreen(&nInsertionLine);

    nStart = GetInsertionCol();
    nEnd = strlen(pL3->m_vecLine[nInsertionLine].szLine);

    if (nNumChars == -1)
    {
        nNumChars = 1;
    }
    memmove(pL3->m_vecLine[nInsertionLine].szLine + nStart, 
            pL3->m_vecLine[nInsertionLine].szLine + nStart + nNumChars, nEnd - (nStart + nNumChars) + 1);

    sTextRendition = pL3->GetDefaultRendition();
    pL3->UpdateRendition(nEnd - nNumChars, nInsertionLine, nNumChars, &sTextRendition);
    return true;
}

bool CScrollBufL2::ProcessInsertChar(int nNumChars)
{
    long nInsertionLine = GetInsertionLine();
    int nStart, nEnd;
    CScrollBufL3 *pL3;

    pL3 = GetScreen(&nInsertionLine);

    nStart = GetInsertionCol();
    nEnd = strlen(pL3->m_vecLine[nInsertionLine].szLine);
    memmove(pL3->m_vecLine[nInsertionLine].szLine + (nStart + nNumChars), pL3->m_vecLine[nInsertionLine].szLine + nStart, nEnd - nStart);
    memmove(pL3->m_vecLine[nInsertionLine].sTextRendition + (nStart + nNumChars), pL3->m_vecLine[nInsertionLine].sTextRendition + nStart, nEnd - nStart);
    return true;
}

void CScrollBufL2::AllocateLines(int nInsertionLine, CScrollBufL3 *pL3)
{
    m_nInsertionLine -= pL3->AllocateLines(nInsertionLine);
    if (m_nInsertionLine < 0)
    {
        m_nInsertionLine = 0;
    }
}

void CScrollBufL2::SaveCoords()
{
    m_nSaveInsertionCol = m_nInsertionCol;
    m_nSaveInsertionLine = GetInsertionLine();
}

void CScrollBufL2::RestoreCoords()
{
    m_nInsertionCol = m_nSaveInsertionCol;
    SetInsertionLine(m_nSaveInsertionLine, false);
}

void CScrollBufL2::InsertNewline()
{
    SetInsertionCol(0);
    SetInsertionLine(GetInsertionLine() + 1, true);
}

/*********************************************************************
**
**  CScrollBufL3
**
*********************************************************************/



CScrollBufL3::CScrollBufL3()
{
    m_nMaxNumLines = 0;
    m_vecLine.clear();
    m_vecDeletedLine.clear();
}

void CScrollBufL3::SetMaxNumLines(long nMaxNumLines)
{
    m_nMaxNumLines = nMaxNumLines;
}

void CScrollBufL3::ClearLine(int y)
{
    STextRendition sTextRendition;

    memset(m_vecLine[y].szLine, 0, sizeof(m_vecLine[y].szLine));
    sTextRendition = GetDefaultRendition();
    UpdateRendition(0, y, SB_NUM_COLS, &sTextRendition);
}

STextRendition CScrollBufL3::GetDefaultRendition()
{
    STextRendition sTextRendition;
    sTextRendition.nAttributes = 0;
    sTextRendition.crBack = SCROLLBUFF_DEFAULT_CR;
    sTextRendition.crText = SCROLLBUFF_DEFAULT_CR;
    return sTextRendition;
}

bool CScrollBufL3::UpdateRendition(int x, int y, int nLen, STextRendition *sTextRendition)
{
    int nIndex;
    if (x < 0)
    {
        TRACE("PROBLEM\n");
    }
    for (nIndex = x; nIndex < (x + nLen); nIndex++)
    {
        m_vecLine[y].sTextRendition[nIndex] = (*sTextRendition);
    }
    return true;
}

long CScrollBufL3::RemoveLines()
{
    long nIndex;
    long nRet = 0;
    for (nIndex = GetMaxNumLines(); nIndex < (long)m_vecLine.size(); nIndex++)
    {
        m_vecDeletedLine.push_back(m_vecLine[0]);
        m_vecLine.pop_front();
        nRet++;
    }
    return nRet;
}

long CScrollBufL3::AllocateLines(long nInsertionLine)
{
    long nIndex;
    /*VerifyCritical();*/
    for (nIndex = m_vecLine.size() - 1; nIndex < nInsertionLine; nIndex++)
    {
        SFullLine sFullLine;
        m_vecLine.push_back(sFullLine);
        ClearLine(m_vecLine.size() - 1);
    }
    return RemoveLines();
}

long CScrollBufL3::GetNumLines()
{
    /*VerifyCritical();*/
    return m_vecLine.size();
}

void CScrollBufL3::ClearAllLines()
{
    m_vecLine.clear();
}
