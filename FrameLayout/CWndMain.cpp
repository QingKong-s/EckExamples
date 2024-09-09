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

	m_Lyt.LoInitDpi(m_iDpi);
	m_Tab.Create(NULL, WS_CHILD | WS_VISIBLE, 0,
		0, 0, 0, 0, hWnd, 0);
	EckCounter(8, i)
	{
		m_Tab.InsertItem(eck::Format(L"Tab %d", i + 1).Data());
		const auto p{ new eck::CPushButton{} };
		p->Create(eck::Format(L"Button %d", i + 1).Data(), WS_CHILD | WS_VISIBLE, 0,
			0, 0, 0, 0, m_Tab.HWnd, 0);
		m_Lyt.Add(p,
			{ .cxLeftWidth = eck::DpiScale(10,m_iDpi),.cxRightWidth = eck::DpiScale(10,m_iDpi) },
			eck::LF_FILL);
		m_vBtn.push_back(p);
	}

	GetSignal().Connect([this](HWND, UINT uMsg, WPARAM, LPARAM lParam, BOOL&)->LRESULT
		{
			if (uMsg == WM_SIZE)
			{
				const int cx = LOWORD(lParam);
				const int cy = HIWORD(lParam);
				SetWindowPos(m_Tab.HWnd, NULL,
					0, 0, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
				RECT rc{ 0,0,cx,cy };
				m_Tab.AdjustRect(&rc, FALSE);
				m_Lyt.Arrange(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
			}
			else if (uMsg == WM_NOTIFY)
			{
				const auto* const pnm = (NMHDR*)lParam;
				if (pnm->code == TCN_SELCHANGE && pnm->hwndFrom == m_Tab.HWnd)
					m_Lyt.ShowFrame(m_Tab.GetCurSel());
			}
			return 0;
		}, eck::MsgHookIdUserBegin);
	m_Lyt.ShowFrame(m_Tab.GetCurSel());
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