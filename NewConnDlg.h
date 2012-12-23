#pragma once
#include "afxwin.h"
#include <vector>
#include "ComboBoxBold.h"
#include "TgtIntf.h"

// CNewConnDlg dialog
#define DCOM_DEBUG_ID     "DEBUG"
#define DCOM_TELNET_ID    "TELNET"
#define DCOM_SSH_ID       "SSH"
#define DCOM_SERIAL_ID    "SERIAL"

#define DCOM_DD_INVALID   0x0000
#define DCOM_DD_TELNET    0x0001 /* different entry types for the dialing dir */
#define DCOM_DD_SSH       0x0002
#define DCOM_DD_DEBUG     0x0003
#define DCOM_DD_SERIAL    0x0004

/*
** If SdialingDir changes then the 
** v# of DCOM_CONNECTION_DATA 
** should also change
*/
struct SDialingDir
{
    int nType;
    int nPort;
    char szAddress[64];
    char reserved0[64];
    char szDescription[64];
    char reserved1[64];
    unsigned long m_dwBaudRate;
    unsigned char m_byParity;
    unsigned char m_byStopBits;
    unsigned char m_byByteSize;
};

#include "DialingDirDlg.h"

class CNewConnDlg : public CSizeDialog
{
	DECLARE_DYNAMIC(CNewConnDlg)
    DECLARE_EASYSIZE

public:
	CNewConnDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNewConnDlg();

// Dialog Data
	enum { IDD = IDD_NEW_CONN_DIALOG };

    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedSshRdo();
    afx_msg void OnBnClickedTelnetRdo();
    afx_msg void OnBnClickedSerialRdo();
    afx_msg void OnBnClickedOk();
    afx_msg void OnCbnSelchangeHostnameCombo();
    afx_msg void OnBnClickedDialingDirButton();
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    char *GetSectionName();

    SDialingDir m_sDialingDir;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual void DetectComPorts();
    virtual LRESULT OnDialingDirOK(WPARAM, LPARAM);
    HICON ConvertBitmapToIcon(int idBitMap);
    void ParseConnectionData();
    bool GetDataFromControls(SDialingDir *sDialingDir);
    bool GetDataFromSerialControls(SDialingDir *sDialingDir);
    SDialingDir *GetDataEntryFromRegistry(int nRegIndex);
    void AddNewRegistryData(SDialingDir *sNewDialingDir);
    void PopulateControls(SDialingDir *sDialingDir);
    void UpdateCombos(SDialingDir *sDialingDir);
    //virtual int GetHelpId() {return IDH_CONNECT;};

	DECLARE_MESSAGE_MAP()
    CComboBox m_commCombo;
    CComboBox m_parityCombo;
    CComboBox m_byteSizeCombo;
    CComboBox m_stopBitsCombo;
    CComboBox m_baudCombo;
    CButton m_sshRdo;
    CButton m_telnetRdo;
    CButton m_serialRdo;
    CButton m_debugRdo;
    CComboBoxBold m_hostNameCombo;
    CEdit m_portEdit;
    std::vector<SDialingDir> m_sDialingDirVec;
    CDialingDirDlg m_dialingDirDlg;
    SDialingDir m_sDialingDirSelection;

};
