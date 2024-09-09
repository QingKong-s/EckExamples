#include "pch.h"

#include "CWndMain.h"

void CWndMain::UpdateDpi(int iDpi)
{
	const int iDpiOld = m_iDpi;
	UpdateDpiInit(iDpi);

	// TODO: 更新字体大小
	HFONT hFont = eck::ReCreateFontForDpiChanged(m_hFont, iDpi, iDpiOld);
	eck::SetFontForWndAndCtrl(HWnd, hFont, FALSE);
	std::swap(m_hFont, hFont);
	DeleteObject(hFont);

	UpdateFixedUISize();
}

void CWndMain::UpdateFixedUISize()
{
}

void CWndMain::ClearRes()
{
	DeleteObject(m_hFont);
}

BOOL CWndMain::OnCreate(HWND hWnd, CREATESTRUCT* lpCreateStruct)
{
	eck::GetThreadCtx()->UpdateDefColor();

	UpdateDpiInit(eck::GetDpi(hWnd));
	m_hFont = eck::EzFont(L"微软雅黑", 9);

	UINT u;
	m_Lyt.LoInitDpi(m_iDpi);
	EckCounter(20, i)
	{
		const auto p{ new eck::CPushButton{} };
		auto rsText = eck::Format(L"Button %d", i);
		if (i > 5 && i < 10)
		{
			u = eck::LF_ALIGN_CENTER | eck::LF_FIX;
			rsText.PushBack(L"[C]");
		}
		else
			u = eck::LF_FIX;
		if (i == 15)
		{
			rsText = L"插入换行测试";
			u |= eck::FLF_BREAKLINE;
		}
		p->Create(rsText.Data(), WS_CHILD | WS_VISIBLE, 0,
			0, 0, eck::DpiScale(100, m_iDpi), eck::DpiScale(eck::Rand(100, 150), m_iDpi), hWnd, 0);
		m_Lyt.Add(p, { .cxLeftWidth = eck::DpiScale(10, m_iDpi) }, u);
		m_vBtn.push_back(p);
	}

	eck::SetFontForWndAndCtrl(hWnd, m_hFont);
	return TRUE;
}

CWndMain::~CWndMain()
{

}

LRESULT CWndMain::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SIZE:
	{
		const auto cx = LOWORD(lParam);
		const auto cy = HIWORD(lParam);
		m_Lyt.Arrange(cx, cy);
	}
	return 0;
	case WM_CREATE:
		return HANDLE_WM_CREATE(hWnd, wParam, lParam, OnCreate);
	case WM_DESTROY:
		ClearRes();
		PostQuitMessage(0);
		return 0;
	case WM_SETTINGCHANGE:
		if (eck::MsgOnSettingChangeMainWnd(hWnd, wParam, lParam))
			Redraw();
		break;
	case WM_DPICHANGED:
		UpdateDpi(HIWORD(wParam));
		m_Lyt.LoOnDpiChanged(m_iDpi);
		eck::MsgOnDpiChanged(hWnd, lParam);
		return 0;
	}
	return CForm::OnMsg(hWnd, uMsg, wParam, lParam);
}

BOOL CWndMain::PreTranslateMessage(const MSG& Msg)
{
	return CForm::PreTranslateMessage(Msg);
}

HWND CWndMain::Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
	int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData)
{
	IntCreate(dwExStyle, WCN_MAIN, pszText, dwStyle,
		x, y, cx, cy, hParent, hMenu, eck::g_hInstance, NULL);
	return NULL;
}