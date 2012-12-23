#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "SizeDialog.h"
#include <map>
#include <string>
#include "CharMapDlg.h"

// CConfigDlg dialog

/* 
** Any time the SDComCfg struct 
** changes the DCOM_SETTINGS_CONFIG v# 
** should also change 
*/

#define CONFIG_DLG_NUM_COLORS   8
struct SDComCfg
{
    COLORREF m_crFore;
    COLORREF m_crBack;
    LOGFONT m_logfont;
    long m_nNumLines;
    int m_nEmulation;
    int m_nWinWidth;
};

struct SKeyCodes
{
    int m_nKey;
    char m_szNormalKey[32];
    char m_szCtrlKey[32];
    char m_szShiftKey[32];
    char m_szDesc[32];
    bool m_bValid;
};

// CColorButton

class CColorButton : public CButton
{
	DECLARE_DYNAMIC(CColorButton)

public:
	CColorButton();
	virtual ~CColorButton();
    void SetColor(COLORREF color);
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

protected:
	DECLARE_MESSAGE_MAP()
    COLORREF m_crBack;
};

class CConfigDlg : public CSizeDialog
{
    DECLARE_DYNAMIC(CConfigDlg)
    DECLARE_EASYSIZE

public:
	CConfigDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CConfigDlg();
	enum { IDD = IDD_CONFIG_DIALOG };
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedFGBlack  ();
    afx_msg void OnBnClickedFGRed    ();
    afx_msg void OnBnClickedFGGreen  ();
    afx_msg void OnBnClickedFGYellow ();
    afx_msg void OnBnClickedFGBlue   ();
    afx_msg void OnBnClickedFGMagenta();
    afx_msg void OnBnClickedFGCyan   ();
    afx_msg void OnBnClickedFGWhite  ();
    afx_msg void OnBnClickedBGBlack  ();
    afx_msg void OnBnClickedBGRed    ();
    afx_msg void OnBnClickedBGGreen  ();
    afx_msg void OnBnClickedBGYellow ();
    afx_msg void OnBnClickedBGBlue   ();
    afx_msg void OnBnClickedBGMagenta();
    afx_msg void OnBnClickedBGCyan   ();
    afx_msg void OnBnClickedBGWhite  ();
    afx_msg void OnBnClickedF1();
    afx_msg void OnBnClickedF10();
    afx_msg void OnBnClickedF11();
    afx_msg void OnBnClickedF12();
    afx_msg void OnBnClickedF2();
    afx_msg void OnBnClickedF3();
    afx_msg void OnBnClickedF4();
    afx_msg void OnBnClickedF5();
    afx_msg void OnBnClickedF6();
    afx_msg void OnBnClickedF7();
    afx_msg void OnBnClickedF8();
    afx_msg void OnBnClickedF9();
    afx_msg void OnBnClickedKeyBackspace();
    afx_msg void OnBnClickedKeyDelete();
    afx_msg void OnBnClickedKeyDown();
    afx_msg void OnBnClickedKeyEnd();
    afx_msg void OnBnClickedKeyHome();
    afx_msg void OnBnClickedKeyInsert();
    afx_msg void OnBnClickedKeyLeft();
    afx_msg void OnBnClickedKeyPgdn();
    afx_msg void OnBnClickedKeyPgup();
    afx_msg void OnBnClickedKeyRight();
    afx_msg void OnBnClickedKeyUp();
    afx_msg void OnBnClickedNormalButton();
    afx_msg void OnBnClickedCtrlButton();
    afx_msg void OnBnClickedShiftButton();
    afx_msg void OnBnClickedDefaults();
    LRESULT OnCharMapOK(WPARAM, LPARAM);
    int GetEmulation();
    void ReadCfg();
    virtual BOOL OnInitDialog();
    SDComCfg *GetConfig();
    virtual char *GetSectionName();
    char *GetKeyString(int nVirtKey, bool bCtrl, bool bShift);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual void RenderSample();
    virtual void SetupColors(CStatic *pStatic, CColorButton *colorButtons, bool bFore);
    virtual void OnBnClickedColor(UINT nId);
    virtual void WriteKeysToRegistry();
    virtual bool ReadKeysFromRegistry();
    virtual void DefaultKeys();
    virtual void SetupKey(int nButton, char *pNormal, char *pCtrl, char *pShift, char *pDesc = NULL);
    virtual void OnBnClicked(int nButton);
    virtual void PopulateControls();
    void OnBnClickedCharMapButton(CEdit *pEdit);
//    virtual int GetHelpId() {return IDH_CONFIGURE;};

	DECLARE_MESSAGE_MAP()
    CColorDialog m_colorDlg;
    CRichEditCtrl m_sampleText;
    SDComCfg m_sDComCfgTmp;
    SDComCfg m_sDComCfgReal;
    bool m_bDComCfgValid;
    CEdit m_scrollbackSizeEdit;
    CColorButton m_foreColors[CONFIG_DLG_NUM_COLORS];
    CColorButton m_backColors[CONFIG_DLG_NUM_COLORS];
    CStatic m_foreStatic;
    CStatic m_backStatic;
    std::map<const int, SKeyCodes> m_keyMap;
    std::map<const int, SKeyCodes> m_tmpKeyMap;
    int m_nCurrentButton;
    CStatic m_sSelectionStatic;
    CButton m_emuRdo;
    CButton m_skipRdo;
    CButton m_passRdo;
    CEdit m_cNormalEdit;
    CButton m_cNormalButton;
    CEdit m_cCtrlEdit;
    CButton m_cCtrlButton;
    CEdit m_cShiftEdit;
    CButton m_cShiftButton;
    CEdit *m_pCharMapEdit;
    CComboBox m_winWidthCombo;
    CComboBox m_cFontSizeCombo;
public:
//    afx_msg void OnCbnSelchangeFontSize();
    afx_msg void OnCbnSelchangeFontSize();
};
