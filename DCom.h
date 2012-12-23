// DCom.h : main header file for the DCom application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

#define DCOM_FILES_DIALING_DIR        ".tdd"
#define DCOM_DIALINGDIR_EXTS          "DCom Dialing Dir (*.tdd)|*.tdd|All Files (*.*)|*.*||"

#define DCOM_FILES_TERMINAL_SESSION   ".tts"
#define DCOM_TERMINALS_EXTS           "DCom Terminal Session (*.tts)|*.tts|All Files (*.*)|*.*||"

// CDComApp:
// See DCom.cpp for the implementation of this class
//

class CCustomCommandLineInfo : public CCommandLineInfo
{
public:
    virtual void ParseParam(const char* pszParam, BOOL bFlag, BOOL bLast);
    enum 
    {
        eFileOpenNothing,
        eFileOpenDialingDir,
        eFileOpenTerminalSession
    } m_nCustomShellCommand;
    CString m_strDialingDirFileName;
    CString m_strTerminalSessionFileName;
};

class CDComApp : public CWinApp
{
public:
	CDComApp();
	virtual BOOL InitInstance();
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

protected:
    void RegAssociateFiles(LPCTSTR lpSubKey);
    BOOL RegKeyExists(LPCTSTR lpSubKey);
    void RegCreateAction();
	BOOL ProcessCustomShellCommands(CCustomCommandLineInfo& rCmdInfo);

};

void ShowHelp(int nHelpId);

extern CDComApp theApp;