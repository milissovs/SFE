#ifndef TPANE_H_INCLUDED
#define TPANE_H_INCLUDED

#include <commctrl.h>
#include <d2d1.h>
#include <dwrite.h>
//#include <uxtheme.h>
#include "btnpane.h"
#include "breadcrumb.h"

class CTPane
{
public:
	CTPane();
	~CTPane();

	virtual LRESULT WindowProcedure(HWND, UINT, WPARAM, LPARAM);

	virtual HWND Create(HWND hWndParent, HINSTANCE hInstance, LPVOID lpParam);

	void SetHeight(int nHeight);
	int  GetHeight() { return (int)(m_nHeight); };

	virtual int      OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct);
	virtual void     OnSize(UINT nType, UINT nWidth, UINT nHeight);

	HWND m_hWnd;
	HWND m_hWndTT;

protected:
	virtual HRESULT OnPaint(HDC hDC, RECT rect);

private:
	float m_fScale;
	INT  m_nHeight;

private:
	CBTNPane    m_wndBtnPane;
	CBreadCrumb m_wndBC;
};

#endif // TPANE_H_INCLUDED
