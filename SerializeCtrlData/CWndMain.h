#pragma once
#include "CApp.h"

class CWndMain : public eck::CForm
{
private:
	eck::CMenu m_Menu{};
	eck::CButton m_BTTest{};
	eck::CTab m_Tab{};
	eck::CFrameLayout m_Lyt{};
	eck::CLinearLayoutH m_LytCtrl{};
	eck::CLinearLayoutV m_LytMain{};

	eck::CButton m_BTPush{}, m_BTRadio{}, m_BTCheck{}, m_BTGroup{}, m_BT3State{}, m_BTCmdLink{};
	eck::CComboBox m_CBBSimple{}, m_CBBDropDown{}, m_CBBDropDownList{};
	eck::CComboBoxEx m_CBE{};
	eck::CComboBoxNew m_CBN{};

	eck::CButton m_BTInputBox{}, m_BTTaskDialog{};

	eck::CLabel m_LA{};

	eck::CListBoxNew m_LBN{};

	std::vector<eck::ILayout*> m_vLyt{};

	HIMAGELIST m_hIL{}, m_hILSmall{};
	struct ITEM
	{
		eck::CRefStrW rsText;
		int idxImage;
	};
	std::vector<ITEM> m_vItem{};

	HFONT m_hFont{};

	int m_iDpi = USER_DEFAULT_SCREEN_DPI;
	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(cxBtn, 180)
		ECK_DS_ENTRY(cyBtn, 50)
		ECK_DS_ENTRY(cxCombo, 180)
		ECK_DS_ENTRY(cyCombo, 30)
		ECK_DS_ENTRY(cxTestBtn, 100)
		ECK_DS_ENTRY(cyTestBtn, 30)
		ECK_DS_ENTRY(cxLB, 200)
		ECK_DS_ENTRY(cyLB, 320)
		ECK_DS_ENTRY(Margin, 10)
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

	BOOL OnCreate(HWND hWnd, CREATESTRUCT* pcs);

	void TestButton();

	void TestComboBox();

	void TestTaskDialog();
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