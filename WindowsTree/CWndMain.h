#pragma once
#include "CApp.h"

class CWndMain : public eck::CForm
{
private:
	eck::CButton m_BTRefresh{};
	eck::CLayoutDummy m_LDummy{};
	eck::CEditExt m_EDFilter{};
	eck::CTreeList m_TL{};

	eck::CLinearLayoutH m_LytCtrl{};
	eck::CLinearLayoutV m_Lyt{};

	struct WNDDATA
	{
		eck::TLNODE Node{};
		HWND hWnd{};
		eck::CRefStrW rs[3];
		std::vector<WNDDATA*> Children{};
	};

	eck::CFixedMemorySet<WNDDATA> wdbuf{};
	HIMAGELIST m_hImgList{};
	std::vector<WNDDATA*> data{};
	WNDDATA* m_Root[2]{};

	void EnumWnd(HWND hWnd, WNDDATA* data, std::vector<WNDDATA*>& fvec);

	void EnumWnd();

	void RefreshTree();

	HFONT m_hFont{};

	int m_iDpi = USER_DEFAULT_SCREEN_DPI;
	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(cxBtn, 60)
		ECK_DS_ENTRY(cyCtrl, 30)
		ECK_DS_ENTRY(cxEdit, 200)
		ECK_DS_ENTRY(cxColCls, 160)
		ECK_DS_ENTRY(cxColHandle, 80)
		ECK_DS_ENTRY(cxColName, 160)
		ECK_DS_ENTRY(cxColThr, 60)
		ECK_DS_ENTRY(cxColProc, 100)
		;
	ECK_DS_END_VAR(m_Ds)
private:
	EckInline void UpdateDpiInit(int iDpi)
	{
		m_iDpi = iDpi;
		eck::UpdateDpiSize(m_Ds, iDpi);
	}

	void UpdateDpi(int iDpi);

	void UpdateFixedUISize();

	void ClearRes();

	BOOL OnCreate(HWND hWnd, CREATESTRUCT* lpCreateStruct);

public:
	ECK_CWND_SINGLEOWNER(CWndMain);

	static ATOM RegisterWndClass()
	{
		return eck::EzRegisterWndClass(WCN_MAIN, eck::CS_STDWND);
	}

	~CWndMain();

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	BOOL PreTranslateMessage(const MSG& Msg) override;

	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override;
};