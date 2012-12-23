#if !defined(AFX_COMBOBOXBOLD_H__D3478A87_E84A_4351_A136_432E41DB0929__INCLUDED_)
#define AFX_COMBOBOXBOLD_H__D3478A87_E84A_4351_A136_432E41DB0929__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ComboBoxBold.h : header file
//

#include <afxtempl.h>


struct ITEMDATA
{
	HICON icon;
    DWORD_PTR pItemData;
};
/////////////////////////////////////////////////////////////////////////////
// CComboBoxBold window

class CComboBoxBold : public CComboBox
{
public:
	int AddString(LPCTSTR lpszString);
	DWORD_PTR GetItemUserData(int nIndex) const;
	int SetItemUserData(int nIndex, DWORD_PTR dwItemData);
	void SetIcon(int iItem, int iconId);
    void SetBitmap(int iItem, int bitmapId);
	void SetIcon(int iItem, HICON hIcon);
	virtual ~CComboBoxBold();
	CComboBoxBold();

	// Generated message map functions
protected:
	virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
    HICON ConvertBitmapToIcon(int idBitMap);

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnDestroy();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMBOBOXBOLD_H__D3478A87_E84A_4351_A136_432E41DB0929__INCLUDED_)
