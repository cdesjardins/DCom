#pragma once
#include <afxwin.h>

class CMyMemDC : public CDC 
{
private:       
    CBitmap    m_bitmap;        // Offscreen bitmap
    CBitmap*   m_oldBitmap;     // bitmap originally found in CMyMemDC
    CDC*       m_pDC;           // Saves CDC passed in constructor
    CRect      m_rect;          // Rectangle of drawing area.
    BOOL       m_bMemDC;        // TRUE if CDC really is a Memory DC.
public:
    CMyMemDC(CDC* pDC, const CRect* pRect = NULL);
    
    ~CMyMemDC();

    // Allow usage as a pointer    
    CMyMemDC* operator->() 
    {
        return this;
    }       
 
    // Allow usage as a pointer    
    operator CMyMemDC*() 
    {
        return this;
    }
};