#pragma once
#include "CApp.h"

class CWndMain : public eck::CForm
{
private:
	struct ITEM
	{
		eck::CRefStrW rsText;
		int idxImage;
		int iIndent;
	};
	eck::CLinearLayoutV m_Lyt{};
	eck::CLinearLayoutH m_LytH1{};
	eck::CLinearLayoutH m_LytH2{};
	eck::CListViewExt m_LV{};
	eck::CListViewExt m_LVOd{};
	eck::CListViewExt m_LVGroup{};
	eck::CListViewExt m_LVTile{};
	std::vector<ITEM> m_vItem{};

	HFONT m_hFont{};

	int m_iDpi = USER_DEFAULT_SCREEN_DPI;
	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(cx, 300)
		ECK_DS_ENTRY(cy, 400)
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

	void InitLvOd();

	void InitLvGroup();

	void InitLvTile();
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