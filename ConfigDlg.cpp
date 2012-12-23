// ConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DCom.h"
#include "MainFrm.h"
#include "ConfigDlg.h"
#include "ScrollBuff.h"

#define DCOM_SETTINGS_SECTION     "Settings"
#define DCOM_SETTINGS_CONFIG      "Config v1"
#define DCOM_SETTINGS_KEY_CONFIG  "%i KeyConfig v1"

// CConfigDlg dialog

struct SColorButtons
{
    UINT nForeId;
    UINT nBackId;
    COLORREF crColor;
};

SColorButtons sColorMap[CONFIG_DLG_NUM_COLORS] = 
{
    { ID_CONFIG_FG_BLACK,   ID_CONFIG_BG_BLACK,   SCROLLBUFF_BLACK_CR  },
    { ID_CONFIG_FG_RED,     ID_CONFIG_BG_RED,     SCROLLBUFF_RED_CR    },
    { ID_CONFIG_FG_GREEN,   ID_CONFIG_BG_GREEN,   SCROLLBUFF_GREEN_CR  },
    { ID_CONFIG_FG_YELLOW,  ID_CONFIG_BG_YELLOW,  SCROLLBUFF_YELLOW_CR },
    { ID_CONFIG_FG_BLUE,    ID_CONFIG_BG_BLUE,    SCROLLBUFF_BLUE_CR   },
    { ID_CONFIG_FG_MAGENTA, ID_CONFIG_BG_MAGENTA, SCROLLBUFF_MAGENTA_CR},
    { ID_CONFIG_FG_CYAN,    ID_CONFIG_BG_CYAN,    SCROLLBUFF_CYAN_CR   },
    { ID_CONFIG_FG_WHITE,   ID_CONFIG_BG_WHITE,   SCROLLBUFF_WHITE_CR  },
};

IMPLEMENT_DYNAMIC(CConfigDlg, CSizeDialog)

CConfigDlg::CConfigDlg(CWnd* pParent /*=NULL*/)
	: CSizeDialog(CConfigDlg::IDD, pParent)
{
    m_bDComCfgValid = false;
    m_nCurrentButton = VK_F1;
    if (ReadKeysFromRegistry() == false)
    {
        DefaultKeys();
    }
}

CConfigDlg::~CConfigDlg()
{
}

SDComCfg * CConfigDlg::GetConfig()
{
    ReadCfg();

    return &m_sDComCfgReal;
}

void CConfigDlg::DoDataExchange(CDataExchange* pDX)
{
    CSizeDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SAMPLE_TEXT, m_sampleText);
    DDX_Control(pDX, IDC_SCROLLBACK_SIZE, m_scrollbackSizeEdit);
    DDX_Control(pDX, IDC_FOREGROUND, m_foreStatic);
    DDX_Control(pDX, IDC_BACKGROUND, m_backStatic);
    DDX_Control(pDX, IDC_SELECTION, m_sSelectionStatic);
    DDX_Control(pDX, IDC_EMM_RDO, m_emuRdo);
    DDX_Control(pDX, IDC_SKIP_RDO, m_skipRdo);
    DDX_Control(pDX, IDC_PASS_RDO, m_passRdo);
    DDX_Control(pDX, IDC_NORMAL_EDIT, m_cNormalEdit);
    DDX_Control(pDX, IDC_NORMAL_BUTTON, m_cNormalButton);
    DDX_Control(pDX, IDC_CTRL_EDIT, m_cCtrlEdit);
    DDX_Control(pDX, IDC_CTRL_BUTTON, m_cCtrlButton);
    DDX_Control(pDX, IDC_SHIFT_EDIT, m_cShiftEdit);
    DDX_Control(pDX, IDC_SHIFT_BUTTON, m_cShiftButton);
    DDX_Control(pDX, IDC_WIN_WIDTH, m_winWidthCombo);
    DDX_Control(pDX, IDC_FONT_SIZE, m_cFontSizeCombo);
}


BEGIN_MESSAGE_MAP(CConfigDlg, CSizeDialog)
    ON_BN_CLICKED(IDOK, &CConfigDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDC_F1, &CConfigDlg::OnBnClickedF1)
    ON_BN_CLICKED(IDC_F10, &CConfigDlg::OnBnClickedF10)
    ON_BN_CLICKED(IDC_F11, &CConfigDlg::OnBnClickedF11)
    ON_BN_CLICKED(IDC_F12, &CConfigDlg::OnBnClickedF12)
    ON_BN_CLICKED(IDC_F2, &CConfigDlg::OnBnClickedF2)
    ON_BN_CLICKED(IDC_F3, &CConfigDlg::OnBnClickedF3)
    ON_BN_CLICKED(IDC_F4, &CConfigDlg::OnBnClickedF4)
    ON_BN_CLICKED(IDC_F5, &CConfigDlg::OnBnClickedF5)
    ON_BN_CLICKED(IDC_F6, &CConfigDlg::OnBnClickedF6)
    ON_BN_CLICKED(IDC_F7, &CConfigDlg::OnBnClickedF7)
    ON_BN_CLICKED(IDC_F8, &CConfigDlg::OnBnClickedF8)
    ON_BN_CLICKED(IDC_F9, &CConfigDlg::OnBnClickedF9)
    ON_BN_CLICKED(IDC_KEY_BACKSPACE, &CConfigDlg::OnBnClickedKeyBackspace)
    ON_BN_CLICKED(IDC_KEY_DELETE, &CConfigDlg::OnBnClickedKeyDelete)
    ON_BN_CLICKED(IDC_KEY_DOWN, &CConfigDlg::OnBnClickedKeyDown)
    ON_BN_CLICKED(IDC_KEY_END, &CConfigDlg::OnBnClickedKeyEnd)
    ON_BN_CLICKED(IDC_KEY_HOME, &CConfigDlg::OnBnClickedKeyHome)
    ON_BN_CLICKED(IDC_KEY_INSERT, &CConfigDlg::OnBnClickedKeyInsert)
    ON_BN_CLICKED(IDC_KEY_LEFT, &CConfigDlg::OnBnClickedKeyLeft)
    ON_BN_CLICKED(IDC_KEY_PGDN, &CConfigDlg::OnBnClickedKeyPgdn)
    ON_BN_CLICKED(IDC_KEY_PGUP, &CConfigDlg::OnBnClickedKeyPgup)
    ON_BN_CLICKED(IDC_KEY_RIGHT, &CConfigDlg::OnBnClickedKeyRight)
    ON_BN_CLICKED(IDC_KEY_UP, &CConfigDlg::OnBnClickedKeyUp)
    ON_BN_CLICKED(ID_CONFIG_FG_BLACK  , &CConfigDlg::OnBnClickedFGBlack  )
    ON_BN_CLICKED(ID_CONFIG_FG_RED    , &CConfigDlg::OnBnClickedFGRed    )
    ON_BN_CLICKED(ID_CONFIG_FG_GREEN  , &CConfigDlg::OnBnClickedFGGreen  )
    ON_BN_CLICKED(ID_CONFIG_FG_YELLOW , &CConfigDlg::OnBnClickedFGYellow )
    ON_BN_CLICKED(ID_CONFIG_FG_BLUE   , &CConfigDlg::OnBnClickedFGBlue   )
    ON_BN_CLICKED(ID_CONFIG_FG_MAGENTA, &CConfigDlg::OnBnClickedFGMagenta)
    ON_BN_CLICKED(ID_CONFIG_FG_CYAN   , &CConfigDlg::OnBnClickedFGCyan   )
    ON_BN_CLICKED(ID_CONFIG_FG_WHITE  , &CConfigDlg::OnBnClickedFGWhite  )
    ON_BN_CLICKED(ID_CONFIG_BG_BLACK  , &CConfigDlg::OnBnClickedBGBlack  )
    ON_BN_CLICKED(ID_CONFIG_BG_RED    , &CConfigDlg::OnBnClickedBGRed    )
    ON_BN_CLICKED(ID_CONFIG_BG_GREEN  , &CConfigDlg::OnBnClickedBGGreen  )
    ON_BN_CLICKED(ID_CONFIG_BG_YELLOW , &CConfigDlg::OnBnClickedBGYellow )
    ON_BN_CLICKED(ID_CONFIG_BG_BLUE   , &CConfigDlg::OnBnClickedBGBlue   )
    ON_BN_CLICKED(ID_CONFIG_BG_MAGENTA, &CConfigDlg::OnBnClickedBGMagenta)
    ON_BN_CLICKED(ID_CONFIG_BG_CYAN   , &CConfigDlg::OnBnClickedBGCyan   )
    ON_BN_CLICKED(ID_CONFIG_BG_WHITE  , &CConfigDlg::OnBnClickedBGWhite  )
    ON_BN_CLICKED(IDC_NORMAL_BUTTON, &CConfigDlg::OnBnClickedNormalButton)
    ON_BN_CLICKED(IDC_CTRL_BUTTON, &CConfigDlg::OnBnClickedCtrlButton)
    ON_BN_CLICKED(IDC_SHIFT_BUTTON, &CConfigDlg::OnBnClickedShiftButton)
    ON_MESSAGE(WM_CHARMAP_OK, OnCharMapOK)
    ON_BN_CLICKED(IDC_DEFAULTS, &CConfigDlg::OnBnClickedDefaults)
//    ON_CBN_SELCHANGE(IDC_FONT_SIZE, &CConfigDlg::OnCbnSelchangeFontSize)
ON_CBN_SELCHANGE(IDC_FONT_SIZE, &CConfigDlg::OnCbnSelchangeFontSize)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CConfigDlg)
END_EASYSIZE_MAP


void CConfigDlg::OnBnClickedF1()
{
    OnBnClicked(VK_F1);
}

void CConfigDlg::OnBnClickedF10()
{
    OnBnClicked(VK_F10);
}

void CConfigDlg::OnBnClickedF11()
{
    OnBnClicked(VK_F11);
}

void CConfigDlg::OnBnClickedF12()
{
    OnBnClicked(VK_F12);
}

void CConfigDlg::OnBnClickedF2()
{
    OnBnClicked(VK_F2);
}

void CConfigDlg::OnBnClickedF3()
{
    OnBnClicked(VK_F3);
}

void CConfigDlg::OnBnClickedF4()
{
    OnBnClicked(VK_F4);
}

void CConfigDlg::OnBnClickedF5()
{
    OnBnClicked(VK_F5);
}

void CConfigDlg::OnBnClickedF6()
{
    OnBnClicked(VK_F6);
}

void CConfigDlg::OnBnClickedF7()
{
    OnBnClicked(VK_F7);
}

void CConfigDlg::OnBnClickedF8()
{
    OnBnClicked(VK_F8);
}

void CConfigDlg::OnBnClickedF9()
{
    OnBnClicked(VK_F9);
}

void CConfigDlg::OnBnClickedKeyBackspace()
{
    OnBnClicked(VK_BACK);
}

void CConfigDlg::OnBnClickedKeyDelete()
{
    OnBnClicked(VK_DELETE);
}

void CConfigDlg::OnBnClickedKeyDown()
{
    OnBnClicked(VK_DOWN);
}

void CConfigDlg::OnBnClickedKeyEnd()
{
    OnBnClicked(VK_END);
}

void CConfigDlg::OnBnClickedKeyHome()
{
    OnBnClicked(VK_HOME);
}

void CConfigDlg::OnBnClickedKeyInsert()
{
    OnBnClicked(VK_INSERT);
}

void CConfigDlg::OnBnClickedKeyLeft()
{
    OnBnClicked(VK_LEFT);
}

void CConfigDlg::OnBnClickedKeyPgdn()
{
    OnBnClicked(VK_NEXT);
}

void CConfigDlg::OnBnClickedKeyPgup()
{
    OnBnClicked(VK_PRIOR);
}

void CConfigDlg::OnBnClickedKeyRight()
{
    OnBnClicked(VK_RIGHT);
}

void CConfigDlg::OnBnClickedKeyUp()
{
    OnBnClicked(VK_UP);
}

// CConfigDlg message handlers

char *CConfigDlg::GetSectionName()
{
    return "Configuration";
}

void CConfigDlg::SetupColors(CStatic *pStatic, CColorButton *colorButtons, bool bFore)
{
    CRect staticRect;
    CRect buttonRect;
    CPoint ptDelta;
    int nIndex;
    UINT nId;

    pStatic->GetWindowRect(staticRect);
    ScreenToClient(staticRect);

    ptDelta.x = staticRect.Width() / CONFIG_DLG_NUM_COLORS;
    ptDelta.y = 0;

    buttonRect.bottom = staticRect.bottom;
    buttonRect.top = staticRect.top + (staticRect.Height() / 2);
    buttonRect.left = staticRect.left;
    buttonRect.right = staticRect.left + ptDelta.x;

    for (nIndex = 0; nIndex < CONFIG_DLG_NUM_COLORS; nIndex++)
    {
        if (bFore)
        {
            nId = sColorMap[nIndex].nForeId;
        }
        else
        {
            nId = sColorMap[nIndex].nBackId;
        }
        colorButtons[nIndex].Create(_T("My button"), 
            WS_CHILD | WS_VISIBLE | BS_BITMAP | BS_OWNERDRAW, 
            buttonRect, this, nId);

        colorButtons[nIndex].SetColor(SCROLLBUFF_MAKE_BOLD(sColorMap[nIndex].crColor));
        buttonRect += ptDelta;
    }
}

BOOL CConfigDlg::OnInitDialog()
{
    CSizeDialog::OnInitDialog();

    m_cNormalEdit.LimitText(sizeof(m_tmpKeyMap[m_nCurrentButton].m_szNormalKey));
    m_cCtrlEdit.LimitText(sizeof(m_tmpKeyMap[m_nCurrentButton].m_szCtrlKey));
    m_cShiftEdit.LimitText(sizeof(m_tmpKeyMap[m_nCurrentButton].m_szShiftKey));

    SetupColors(&m_foreStatic, m_foreColors, true);
    SetupColors(&m_backStatic, m_backColors, false);

    GetConfig();
    RenderSample();

    PopulateControls();
    return TRUE;
}

void CConfigDlg::ReadCfg()
{
    SDComCfg *pDComCfg;
    UINT nBytes;
    CFont font;
    if (m_bDComCfgValid == false)
    {
        memset(&m_sDComCfgReal, 0, sizeof(m_sDComCfgReal));
        m_bDComCfgValid = true;
        m_sDComCfgReal.m_crFore = RGB(0xcc, 0xcc, 0xcc);
        m_sDComCfgReal.m_crBack = RGB(0x00, 0x00, 0x00);
        font.CreateFont(12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, _T("Courier"));
        font.GetLogFont(&m_sDComCfgReal.m_logfont);
        m_sDComCfgReal.m_nNumLines = 20000;
        m_sDComCfgReal.m_nWinWidth = 0;

        if (AfxGetApp()->GetProfileBinary(DCOM_SETTINGS_SECTION, 
            DCOM_SETTINGS_CONFIG, (LPBYTE*)&pDComCfg, &nBytes) == TRUE)
        {
            if (nBytes <= sizeof(SDComCfg))
            {
                memcpy(&m_sDComCfgReal, pDComCfg, nBytes);
                if (nBytes < sizeof(SDComCfg))
                {
                    memset(((char*)(&m_sDComCfgReal)) + nBytes, 0, sizeof(SDComCfg) - nBytes);
                }
            }
            delete pDComCfg;
        }
        memcpy(&m_sDComCfgTmp, &m_sDComCfgReal, sizeof(SDComCfg));
    }
}

void CConfigDlg::RenderSample()
{
    CFont font;
    CHARFORMAT2 cf;
    long s1,s2;

    font.CreateFontIndirect(&m_sDComCfgTmp.m_logfont);
    m_sampleText.SetFont(&font);

    memset(&cf, 0, sizeof(cf));
    cf.cbSize = sizeof(cf);
    cf.crTextColor = m_sDComCfgTmp.m_crFore;
    cf.crBackColor = m_sDComCfgTmp.m_crBack;
    cf.dwMask = CFM_COLOR | CFM_BACKCOLOR;

    m_sampleText.SetSel(0, -1);
    m_sampleText.ReplaceSel("Normal");
    m_sampleText.SetSel(0, -1);
    m_sampleText.SetSelectionCharFormat(cf);
    m_sampleText.GetSel(s1, s2);

    cf.crTextColor = SCROLLBUFF_MAKE_BOLD(m_sDComCfgTmp.m_crFore);

    m_sampleText.SetSel(s2, -1);
    m_sampleText.ReplaceSel(" Bold");
    m_sampleText.SetSel(s2, -1);
    m_sampleText.SetSelectionCharFormat(cf);
    m_sampleText.SetSel(-1, -1);

    m_sampleText.SetBackgroundColor(FALSE, m_sDComCfgTmp.m_crBack);
    font.DeleteObject();
}

void CConfigDlg::WriteKeysToRegistry()
{
    char szEntry[32];
    int nIndex = 0;
    std::map<const int, SKeyCodes>::iterator pos;
    SKeyCodes sKeyCodes;

    m_keyMap = m_tmpKeyMap;

    for (pos = m_keyMap.begin(); pos != m_keyMap.end(); pos++)
    {
        sprintf(szEntry, DCOM_SETTINGS_KEY_CONFIG, nIndex++);
        sKeyCodes = pos->second;
        AfxGetApp()->WriteProfileBinary(DCOM_SETTINGS_SECTION, szEntry, (LPBYTE)&sKeyCodes, sizeof(SKeyCodes));
    } 
}

bool CConfigDlg::ReadKeysFromRegistry()
{
    bool bRet = false;
    char szEntry[32];
    int nIndex = 0;
    BOOL bValid = FALSE;
    UINT nSize;
    SKeyCodes *pKeyCodes;
    do
    {
        sprintf(szEntry, DCOM_SETTINGS_KEY_CONFIG, nIndex++);
        bValid = AfxGetApp()->GetProfileBinary(DCOM_SETTINGS_SECTION, szEntry, (LPBYTE*)&pKeyCodes, &nSize);
        if (bValid)
        {
            bRet = true;
            m_tmpKeyMap[pKeyCodes->m_nKey] = *pKeyCodes;
            delete(pKeyCodes);
        }
    } while (bValid);
    m_keyMap = m_tmpKeyMap;
    return bRet;
}

void CConfigDlg::DefaultKeys()
{
    char bs[2] = {8,0};
    SetupKey(VK_F12         , "\033[24~", "",              "",               "F12");
    SetupKey(VK_F11         , "\033[23~", "",              "",               "F11");
    SetupKey(VK_F10         , "\033[21~", "",              "\033[34~",       "F10");
    SetupKey(VK_F9          , "\033[20~", "",              "\033[33~",       "F9 ");
    SetupKey(VK_F8          , "\033[19~", "",              "\033[32~",       "F8 ");
    SetupKey(VK_F7          , "\033[18~", "",              "\033[31~",       "F7 ");
    SetupKey(VK_F6          , "\033[17~", "",              "\033[29~",       "F6 ");
    SetupKey(VK_F5          , "\033[6~" , "\033[[5~",      "\033[28~",       "F5 ");
    SetupKey(VK_F4          , "\033OS"  , "\033[[4~",      "\033[26~",       "F4 ");
    SetupKey(VK_F3          , "\033OR"  , "\033[[3~",      "\033[25~",       "F3 ");
    SetupKey(VK_F2          , "\033OQ"  , "\033[[2~",      "\033[24~",       "F2 ");
    SetupKey(VK_F1          , "\033OP"  , "\033[[1~",      "\033[23~",       "F1 ");
    SetupKey(VK_UP          , "\033[A"  , "\033OA",        "",               "Up    ");
    SetupKey(VK_DOWN        , "\033[B"  , "\033OB",        "",               "Down  ");
    SetupKey(VK_RIGHT       , "\033[C"  , "\033OC",        "",               "Right ");
    SetupKey(VK_LEFT        , "\033[D"  , "\033OD",        "",               "Left  ");
    SetupKey(VK_BACK        , bs        , "\033H",         "",               "Backspace  ");
    SetupKey(VK_HOME        , "\033[1~" , "",              "\033[H",         "Home  ");
    SetupKey(VK_INSERT      , "\033[2~" , "\033[L",        "",               "Insert");
    SetupKey(VK_DELETE      , "\033[3~" , "\033[M",        "",               "Delete");
    SetupKey(VK_END         , "\033[4~" , "\033[K",        "",               "End   ");
    SetupKey(VK_PRIOR       , "\033[5~" , "",              "",               "PageUp");
    SetupKey(VK_NEXT        , "\033[6~" , "\033[H\033[2J", "",               "PageDown");

    m_keyMap = m_tmpKeyMap;
}

char *CConfigDlg::GetKeyString(int nVirtKey, bool bCtrl, bool bShift)
{
    if (m_keyMap[nVirtKey].m_bValid == true)
    {
        if ((bCtrl == true) && (bShift == false))
        {
            return m_keyMap[nVirtKey].m_szCtrlKey;
        }
        else if ((bCtrl == false) && (bShift == true))
        {
            return m_keyMap[nVirtKey].m_szShiftKey;
        }
        else
        {
            return m_keyMap[nVirtKey].m_szNormalKey;
        }
    }
    else
    {
        return NULL;
    }
}

void CConfigDlg::SetupKey(int nButton, char *pNormal, char *pCtrl, char *pShift, char *pDesc)
{
    m_tmpKeyMap[nButton].m_nKey = nButton;
    strncpy(m_tmpKeyMap[nButton].m_szNormalKey, pNormal, sizeof(m_tmpKeyMap[nButton].m_szNormalKey));
    strncpy(m_tmpKeyMap[nButton].m_szCtrlKey, pCtrl, sizeof(m_tmpKeyMap[nButton].m_szCtrlKey));
    strncpy(m_tmpKeyMap[nButton].m_szShiftKey, pShift, sizeof(m_tmpKeyMap[nButton].m_szShiftKey));
    if (pDesc)
    {
        strncpy(m_tmpKeyMap[nButton].m_szDesc, pDesc, sizeof(m_tmpKeyMap[nButton].m_szDesc));
        m_tmpKeyMap[nButton].m_bValid = true;
    }
}

void CConfigDlg::OnBnClickedColor(UINT nId)
{
    int nIndex;
    for (nIndex = 0; nIndex < CONFIG_DLG_NUM_COLORS; nIndex++)
    {
        if (nId == sColorMap[nIndex].nForeId)
        {
            m_sDComCfgTmp.m_crFore = sColorMap[nIndex].crColor;
        }
        else if (nId == sColorMap[nIndex].nBackId)
        {
            m_sDComCfgTmp.m_crBack = sColorMap[nIndex].crColor;
        }
    }
    RenderSample();
}

void CConfigDlg::OnBnClickedFGBlack  ()
{
    OnBnClickedColor(ID_CONFIG_FG_BLACK);
}

void CConfigDlg::OnBnClickedFGRed    ()
{
    OnBnClickedColor(ID_CONFIG_FG_RED);
}

void CConfigDlg::OnBnClickedFGGreen  ()
{
    OnBnClickedColor(ID_CONFIG_FG_GREEN);
}

void CConfigDlg::OnBnClickedFGYellow ()
{
    OnBnClickedColor(ID_CONFIG_FG_YELLOW);
}

void CConfigDlg::OnBnClickedFGBlue   ()
{
    OnBnClickedColor(ID_CONFIG_FG_BLUE);
}

void CConfigDlg::OnBnClickedFGMagenta()
{
    OnBnClickedColor(ID_CONFIG_FG_MAGENTA);
}

void CConfigDlg::OnBnClickedFGCyan   ()
{
    OnBnClickedColor(ID_CONFIG_FG_CYAN);
}

void CConfigDlg::OnBnClickedFGWhite  ()
{
    OnBnClickedColor(ID_CONFIG_FG_WHITE);
}

void CConfigDlg::OnBnClickedBGBlack  ()
{
    OnBnClickedColor(ID_CONFIG_BG_BLACK);
}

void CConfigDlg::OnBnClickedBGRed    ()
{
    OnBnClickedColor(ID_CONFIG_BG_RED);
}

void CConfigDlg::OnBnClickedBGGreen  ()
{
    OnBnClickedColor(ID_CONFIG_BG_GREEN);
}

void CConfigDlg::OnBnClickedBGYellow ()
{
    OnBnClickedColor(ID_CONFIG_BG_YELLOW);
}

void CConfigDlg::OnBnClickedBGBlue   ()
{
    OnBnClickedColor(ID_CONFIG_BG_BLUE);
}

void CConfigDlg::OnBnClickedBGMagenta()
{
    OnBnClickedColor(ID_CONFIG_BG_MAGENTA);
}

void CConfigDlg::OnBnClickedBGCyan   ()
{
    OnBnClickedColor(ID_CONFIG_BG_CYAN);
}

void CConfigDlg::OnBnClickedBGWhite  ()
{
    OnBnClickedColor(ID_CONFIG_BG_WHITE);
}

void CConfigDlg::PopulateControls()
{
    CString szTmp;
    int nCnt;

    m_cFontSizeCombo.SetCurSel(m_sDComCfgTmp.m_logfont.lfHeight - 6);
    m_sSelectionStatic.SetWindowText(m_tmpKeyMap[m_nCurrentButton].m_szDesc);

    m_cNormalEdit.SetWindowText(m_tmpKeyMap[m_nCurrentButton].m_szNormalKey);
    m_cCtrlEdit.SetWindowText(m_tmpKeyMap[m_nCurrentButton].m_szCtrlKey);
    m_cShiftEdit.SetWindowText(m_tmpKeyMap[m_nCurrentButton].m_szShiftKey);

    szTmp.Format("%i", m_sDComCfgTmp.m_nNumLines);
    m_scrollbackSizeEdit.SetWindowText(szTmp);


    for (nCnt = SB_MIN_COLS; nCnt <= SB_NUM_COLS; nCnt++)
    {
        szTmp.Format("%i", nCnt);
        m_winWidthCombo.AddString(szTmp);
    }
    m_winWidthCombo.SetCurSel(m_sDComCfgTmp.m_nWinWidth);

    switch (m_sDComCfgTmp.m_nEmulation)
    {
    case SCROLLBUFF_CFG_EMM_EMMULATE:
        m_emuRdo.SetCheck(BST_CHECKED);
        break;
    case SCROLLBUFF_CFG_EMM_SKIP:
        m_skipRdo.SetCheck(BST_CHECKED);
        break;
    case SCROLLBUFF_CFG_EMM_PASS:
        m_passRdo.SetCheck(BST_CHECKED);
        break;
    }
}

void CConfigDlg::OnBnClicked(int nButton)
{
    CString szNormal;
    CString szCtrl;
    CString szShift;

    m_cNormalEdit.GetWindowText(szNormal);
    m_cCtrlEdit.GetWindowText(szCtrl);
    m_cShiftEdit.GetWindowText(szShift);

    SetupKey(m_nCurrentButton, szNormal.GetBuffer(), szCtrl.GetBuffer(), szShift.GetBuffer());

    m_nCurrentButton = nButton;

    PopulateControls();
}

void CConfigDlg::OnBnClickedOk()
{
    CMainFrame *pMainFrm;
    CString szTmp;

    if (m_emuRdo.GetCheck() == BST_CHECKED)
    {
        m_sDComCfgTmp.m_nEmulation = SCROLLBUFF_CFG_EMM_EMMULATE;
    }
    else if (m_skipRdo.GetCheck() == BST_CHECKED)
    {
        m_sDComCfgTmp.m_nEmulation = SCROLLBUFF_CFG_EMM_SKIP;
    }
    else if (m_passRdo.GetCheck() == BST_CHECKED)
    {
        m_sDComCfgTmp.m_nEmulation = SCROLLBUFF_CFG_EMM_PASS;
    }
    m_scrollbackSizeEdit.GetWindowText(szTmp);
    m_sDComCfgTmp.m_nNumLines = atoi(szTmp.GetBuffer());

    m_sDComCfgTmp.m_nWinWidth = m_winWidthCombo.GetCurSel();

    OnBnClicked(m_nCurrentButton);
    WriteKeysToRegistry();

    memcpy(&m_sDComCfgReal, &m_sDComCfgTmp, sizeof(SDComCfg));

    AfxGetApp()->WriteProfileBinary(DCOM_SETTINGS_SECTION, 
        DCOM_SETTINGS_CONFIG, (LPBYTE)&m_sDComCfgReal, sizeof(SDComCfg));
    pMainFrm = (CMainFrame *)AfxGetMainWnd();
    if (pMainFrm)
    {
        pMainFrm->SetConfig(&m_sDComCfgReal);
        pMainFrm->InvalidateAll();
    }
    
    OnOK();
}

int CConfigDlg::GetEmulation()
{
    return m_sDComCfgReal.m_nEmulation;
}









// colordlg.cpp : implementation file
//


// CColorButton

IMPLEMENT_DYNAMIC(CColorButton, CButton)

CColorButton::CColorButton()
{

}

CColorButton::~CColorButton()
{
}


BEGIN_MESSAGE_MAP(CColorButton, CButton)
END_MESSAGE_MAP()



// CColorButton message handlers

void CColorButton::SetColor(COLORREF color)
{
    m_crBack = color;
}


void CColorButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    CDC dc;
    CRect rt;

    rt = lpDrawItemStruct->rcItem;
    dc.Attach(lpDrawItemStruct->hDC);
    dc.FillSolidRect(rt, m_crBack);

    UINT state = lpDrawItemStruct->itemState;   //Get state of the button
    if ((state & ODS_SELECTED))                 // If it is pressed
    {
        dc.DrawEdge(rt, EDGE_SUNKEN, BF_RECT);  // Draw a sunken face
    }
    else
    {
        dc.DrawEdge(rt, EDGE_RAISED, BF_RECT);  // Draw a raised face
    }

    dc.Detach();
}

void CConfigDlg::OnBnClickedCharMapButton(CEdit *pEdit)
{
    CCharMapDlg charMapDlg;

    m_pCharMapEdit = pEdit;
    charMapDlg.DoModal();
}

LRESULT CConfigDlg::OnCharMapOK(WPARAM ch, LPARAM)
{
    CString szText;
    char chSelected = (char)ch;
    m_pCharMapEdit->GetWindowText(szText);
    szText += chSelected;
    m_pCharMapEdit->SetWindowText(szText);
    return 0;
}

void CConfigDlg::OnBnClickedNormalButton()
{
    OnBnClickedCharMapButton(&m_cNormalEdit);
}

void CConfigDlg::OnBnClickedCtrlButton()
{
    OnBnClickedCharMapButton(&m_cCtrlEdit);
}

void CConfigDlg::OnBnClickedShiftButton()
{
    OnBnClickedCharMapButton(&m_cShiftEdit);
}

void CConfigDlg::OnBnClickedDefaults()
{
    DefaultKeys();
    PopulateControls();
}

void CConfigDlg::OnCbnSelchangeFontSize()
{
    m_sDComCfgTmp.m_logfont.lfHeight = m_cFontSizeCombo.GetCurSel() + 6;
    RenderSample();
}
