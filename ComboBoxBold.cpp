// ComboBoxBold.cpp : implementation file
//

#include "stdafx.h"
#include "ComboBoxBold.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CComboBoxBold

CComboBoxBold::CComboBoxBold()
{
}

CComboBoxBold::~CComboBoxBold()
{

}


BEGIN_MESSAGE_MAP(CComboBoxBold, CComboBox)
	//{{AFX_MSG_MAP(CComboBoxBold)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
    ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CComboBoxBold message handlers
void CComboBoxBold::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{

    if (GetCount() == 0)
    {
        return;
    }
    CString str;
    GetLBText(lpDrawItemStruct->itemID,str);
    CDC dc;

    dc.Attach(lpDrawItemStruct->hDC);

    // Save these value to restore them when done drawing.
    COLORREF crOldTextColor = dc.GetTextColor();
    COLORREF crOldBkColor = dc.GetBkColor();

    // If this item is selected, set the background color 
    // and the text color to appropriate values. Erase
    // the rect by filling it with the background color.
    if ((lpDrawItemStruct->itemAction | ODA_SELECT) &&
        (lpDrawItemStruct->itemState  & ODS_SELECTED))
    {
        dc.SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
        dc.SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
        dc.FillSolidRect(&lpDrawItemStruct->rcItem, ::GetSysColor(COLOR_HIGHLIGHT));
    }
    else
    {
        dc.FillSolidRect(&lpDrawItemStruct->rcItem, crOldBkColor);
    }
    CRect rect(lpDrawItemStruct->rcItem);
    rect.DeflateRect(1,0);

    HICON hIcon = NULL;
    ITEMDATA *iData;
    iData = (ITEMDATA *)GetItemData(lpDrawItemStruct->itemID);
    if ((int)iData != CB_ERR)
    {
        hIcon = iData->icon;
    }

    DrawIconEx(dc.GetSafeHdc(),rect.left,rect.top,hIcon,0, 0, 0, NULL, DI_NORMAL);

    rect.left += 17;

    // Draw the text.
    dc.DrawText(
        str,
        -1,
        &rect,
        DT_LEFT|DT_SINGLELINE|DT_VCENTER);

    // Reset the background color and the text color back to their
    // original values.
    dc.SetTextColor(crOldTextColor);
    dc.SetBkColor(crOldBkColor);

    dc.Detach();
}

void CComboBoxBold::SetIcon(int iItem, HICON hIcon)
{
	ITEMDATA *iData;
    iData = (ITEMDATA *)GetItemData(iItem);
    if ((int)iData != CB_ERR)
    {
       iData->icon = hIcon;
       SetItemData(iItem, (DWORD_PTR)iData);
    }
	Invalidate();
}

void CComboBoxBold::SetIcon(int iItem, int iconId)
{
	HICON hIcon = (HICON)::LoadImage(AfxGetInstanceHandle(),
	MAKEINTRESOURCE(iconId),IMAGE_ICON,16,16,0);

    SetIcon(iItem, hIcon);
}

void CComboBoxBold::SetBitmap(int iItem, int idBitMap)
{
    SetIcon(iItem, ConvertBitmapToIcon(idBitMap));
}

HICON CComboBoxBold::ConvertBitmapToIcon(int idBitMap)
{
    CBitmap bm;
    int nImg;
    HICON hIcon;
    CImageList m_imgIcon;

    bm.LoadBitmap(idBitMap);
    m_imgIcon.Create(16,16, ILC_COLORDDB | ILC_MASK,0,10);
    nImg = m_imgIcon.Add(&bm, RGB(255,255,255));
    hIcon = m_imgIcon.ExtractIcon(nImg);
    return hIcon;
}
DWORD_PTR CComboBoxBold::GetItemUserData(int nIndex) const
{
	ITEMDATA *iData;
    iData = (ITEMDATA *)GetItemData(nIndex);
    return iData->pItemData;
}

int CComboBoxBold::SetItemUserData(int nIndex, DWORD_PTR dwItemData)
{
    ITEMDATA *iData;
    iData = (ITEMDATA *)GetItemData(nIndex);
    if ((int)iData != CB_ERR)
    {
        iData->pItemData = dwItemData;
        return SetItemData(nIndex, (DWORD_PTR)iData);
    }
    else
    {
        return CB_ERR;
    }
}

int CComboBoxBold::AddString(LPCTSTR lpszString)
{
    int nRet;
    ITEMDATA *iData;
    nRet = CComboBox::AddString(lpszString);
    if (nRet >= CB_OKAY)
    {
        iData = new ITEMDATA;
        if (iData)
        {
            iData->icon = 0;
            iData->pItemData = 0;
            SetItemData(nRet, (DWORD_PTR)iData);
        }
    }
    return nRet;
}


void CComboBoxBold::OnDestroy()
{
    int nIndex;
    int nMax;
    ITEMDATA *iData;

    nMax = GetCount();
    for (nIndex = 0; nIndex < nMax; nIndex++)
    {
        iData = (ITEMDATA *)GetItemData(nIndex);
        if (iData)
        {
            delete iData;
        }
    }
    CComboBox::OnDestroy();
}
