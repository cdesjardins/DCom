#pragma once

#include "docs/dcomtopics.h"

// CSizeDialog dialog

class CSizeDialog : public CDialog
{
	DECLARE_DYNAMIC(CSizeDialog)
    virtual void __ES__RepositionControls(BOOL bInit) = 0;
    virtual void __ES__CalcBottomRight(CWnd *pThis, BOOL bBottom, int &bottomright, int &topleft, UINT id, UINT br, int es_br, CRect &rect, int clientbottomright) = 0;

public:    

	CSizeDialog(UINT nIDTemplate, CWnd* pParent = NULL);   // standard constructor
	virtual ~CSizeDialog();
    afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    virtual BOOL OnInitDialog();
	enum { IDD = IDD_SIZEDIALOG };

protected:
    virtual void OnOK();
    virtual char *GetSectionName() = 0;
    virtual int GetHelpId() { return IDH_MAIN; };
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
    CRect m_initialRect;
};
