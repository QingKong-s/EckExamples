#include "pch.h"

#include "CWndMain.h"

void CWndMain::EnumWnd(HWND hWnd, WNDDATA* data, std::vector<WNDDATA*>& fvec)
{
	auto h = GetWindow(hWnd, GW_CHILD);
	while (h)
	{
		//if (IsWindowVisible(h))
		{
			BOOL b;
			auto hicon = eck::GetWindowIcon(h, b, TRUE);
			int idx = -1;
			if (hicon)
				idx = ImageList_AddIcon(m_hImgList, hicon);
			if (b)
				DestroyIcon(hicon);
			//EnumWnd(h, data->Children.emplace_back(new WNDDATA{ {},h }));
			auto p = wdbuf.New(eck::TLNODE{ 0,0,0,idx,-1 }, h);
			p->rs[0].Format(L"0x%08X", h);
			p->rs[1] = eck::CWnd(h).GetClsName();
			p->rs[2] = eck::CWnd(h).GetText();
			fvec.emplace_back(p);
			EnumWnd(h, data->Children.emplace_back(p), fvec);
		}
		h = GetWindow(h, GW_HWNDNEXT);
	}
}

void CWndMain::EnumWnd()
{
	m_hImgList = ImageList_Create(eck::DpiScale(16, m_iDpi), eck::DpiScale(16, m_iDpi),
		ILC_COLOR32 | ILC_ORIGINALSIZE, 0, 40);

	HWND h = GetDesktopWindow();
	BOOL b;
	auto hicon = eck::GetWindowIcon(h, b, TRUE);
	int idx = -1;
	if (hicon)
		idx = ImageList_AddIcon(m_hImgList, hicon);
	if (b)
		DestroyIcon(hicon);

	m_Root[0] = wdbuf.New(eck::TLNODE{ 0,0,0,idx,-1 }, h);
	m_Root[0]->rs[0] = eck::CWnd(h).GetClsName();
	m_Root[0]->rs[1].Format(L"0x%08X", h);
	m_Root[0]->rs[2] = eck::CWnd(h).GetText();
	EnumWnd(GetDesktopWindow(), m_Root[0], data);

	h = HWND_MESSAGE;
	m_Root[1] = wdbuf.New(eck::TLNODE{ 0,0,0,-1,-1 }, h);
	m_Root[1]->rs[0] = L"HWND_MESSAGE";
	m_Root[1]->rs[1].Format(L"0x%08X", (UINT)h);
	m_Root[1]->rs[2] = L"HWND_MESSAGE";

	HWND hMo{};
	while ((hMo = FindWindowExW(HWND_MESSAGE, hMo, 0, 0)))
	{
		BOOL b;
		auto hicon = eck::GetWindowIcon(hMo, b, TRUE);
		int idx = -1;
		if (hicon)
		{
			idx = ImageList_AddIcon(m_hImgList, hicon);
			if (b)
				DestroyIcon(hicon);
		}
		//EnumWnd(h, data->Children.emplace_back(new WNDDATA{ {},h }));
		auto p0 = wdbuf.New(eck::TLNODE{ 0,0,0,idx,-1 }, hMo);
		p0->rs[0] = eck::CWnd(hMo).GetClsName();
		p0->rs[1].Format(L"0x%08X", (UINT)hMo);
		p0->rs[2] = eck::CWnd(hMo).GetText();
		m_Root[1]->Children.push_back(p0);
		data.emplace_back(p0);
		//p->Children.emplace_back(p0);
		//EnumWnd(hMo, p->Children.emplace_back(p0), data);
	}
}

void CWndMain::RefreshTree()
{
	EnumWnd();
	m_TL.SetImageList(m_hImgList);
	m_TL.BuildTree();
}

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
	// TODO: 更新固定大小的控件

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

	{
		m_BTRefresh.Create(L"刷新", WS_CHILD | WS_VISIBLE, 0,
			0, 0, m_Ds.cxBtn, m_Ds.cyCtrl, hWnd, 0);
		m_LytCtrl.Add(&m_BTRefresh, {}, eck::LF_FIX);

		m_LDummy.LoSetSize(500, m_Ds.cyCtrl);
		m_LytCtrl.Add(&m_LDummy, {}, eck::LF_FILL_WIDTH | eck::LF_FIX_HEIGHT, 1u);

		m_EDFilter.Create(nullptr, WS_CHILD | WS_VISIBLE, 0,
			10, 10, m_Ds.cxEdit, m_Ds.cyCtrl, hWnd, 0);
		m_EDFilter.SetFrameType(1);
		m_EDFilter.FrameChanged();
		m_LytCtrl.Add(&m_EDFilter, {}, eck::LF_FIX);
	}
	m_Lyt.Add(&m_LytCtrl, {}, eck::LF_FIX_HEIGHT | eck::LF_FILL_WIDTH);

	m_TL.Create(nullptr, WS_CHILD | WS_VISIBLE | WS_BORDER, 0,
		0, 0, 10, 10, hWnd, 0);
	const auto& Hdr = m_TL.GetHeader();
	Hdr.Style |= HDS_FULLDRAG;
	Hdr.InsertItem(L"窗口类名", -1, m_Ds.cxColCls);
	Hdr.InsertItem(L"窗口句柄", -1, m_Ds.cxColHandle);
	Hdr.InsertItem(L"窗口标题", -1, m_Ds.cxColName);
	Hdr.InsertItem(L"线程ID", -1, m_Ds.cxColThr);
	Hdr.InsertItem(L"进程", -1, m_Ds.cxColProc);

	m_Lyt.Add(&m_TL, {}, eck::LF_FILL, 1u);

	RefreshTree();

	m_Lyt.LoInitDpi(m_iDpi);

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
	case WM_NOTIFY:
	{
		auto pnm = (NMHDR*)lParam;
		if (pnm->hwndFrom == m_TL.HWnd)
		{
			switch (pnm->code)
			{
			case eck::NM_TL_FILLCHILDREN:
			{
				auto p = (eck::NMTLFILLCHILDREN*)lParam;
				if (p->bQueryRoot)
				{
					p->cChildren = 2;
					p->pChildren = (eck::TLNODE**)m_Root;
					/*p->cChildren = (int)data[0]->Children.size();
					p->pChildren = (eck::TLNODE**)data[0]->Children.data();*/
				}
				else
				{
					auto pd = (WNDDATA*)p->pParent;
					p->pChildren = (eck::TLNODE**)pd->Children.data();
					p->cChildren = (int)pd->Children.size();
				}
			}
			return 0;
			case eck::NM_TL_FILLALLFLATITEM:
			{
				auto p = (eck::NMTLFILLALLFLATITEM*)lParam;
				p->cItem = (int)data.size();
				p->pItems = (eck::TLNODE**)data.data();
			}
			return 0;
			case eck::NM_TL_GETDISPINFO:
			{
				auto p = (eck::NMTLGETDISPINFO*)lParam;
				auto pd = (WNDDATA*)p->Item.pNode;
				switch (p->Item.idxSubItem)
				{
				case 0:
					p->Item.pszText = pd->rs[0].Data();
					p->Item.cchText = pd->rs[0].Size();
					break;
				case 1:
					p->Item.pszText = pd->rs[1].Data();
					p->Item.cchText = pd->rs[1].Size();
					break;
				case 2:
					p->Item.pszText = pd->rs[2].Data();
					p->Item.cchText = pd->rs[2].Size();
					break;
				}
			}
			return 0;
			case eck::NM_TL_HD_CLICK:
			{
				auto p = (NMHEADERW*)lParam;

				const int idxCol = p->iItem;
				auto& H = m_TL.GetHeader();
				static int isortorder = 0;
				int fmt;
				if (isortorder == 0)
				{
					isortorder = 1;
					fmt = HDF_SORTUP;
				}
				else if (isortorder == 1)
				{
					isortorder = -1;
					fmt = HDF_SORTDOWN;
				}
				else if (isortorder == -1)
				{
					isortorder = 0;
					fmt = 0;
				}

				H.RadioSetSortMark(idxCol, fmt);

				if (isortorder == 1)
				{
					std::sort(data.begin(), data.end(), [idxCol](const WNDDATA* p1, const WNDDATA* p2)
						{
							return p1->rs[idxCol] < p2->rs[idxCol];
						});
					m_TL.SetFlatMode(TRUE);
				}
				else if (isortorder == -1)
				{
					std::sort(data.begin(), data.end(), [idxCol](const WNDDATA* p1, const WNDDATA* p2)
						{
							return  p2->rs[idxCol] < p1->rs[idxCol];
						});
					m_TL.SetFlatMode(TRUE);
				}
				else
					m_TL.SetFlatMode(FALSE);
				m_TL.BuildTree();
				m_TL.Redraw();
			}
			return 0;
			case NM_CUSTOMDRAW:
			{
				auto p = (eck::NMTLCUSTOMDRAW*)lParam;
				const auto pNode = (WNDDATA*)p->pNode;
				switch (p->dwDrawStage)
				{
				case CDDS_PREPAINT:
					return CDRF_NOTIFYITEMDRAW;
				case CDDS_ITEMPREPAINT:
				{
					if (!IsWindow(pNode->hWnd))
						p->crText = eck::Colorref::Red;
					else if (!IsWindowVisible(pNode->hWnd))
						p->crText = eck::GetThreadCtx()->crGray1;
				}
				return CDRF_DODEFAULT;
				}
			}
			return CDRF_DODEFAULT;

			case eck::NM_TL_PREEDIT:
			{
				EckDbgPrint(L"Pre Edit");
			}
			return 0;
			case eck::NM_TL_POSTEDIT:
			{
				auto p = (eck::NMTLEDIT*)lParam;
				auto& Edit = m_TL.GetEdit();
				EckDbgPrintFmt(L"Post Edit, Text = %s，Changed = %d",
					Edit.GetText().Data(), !!(p->uFlags & eck::TLEDF_SHOULDSAVETEXT));
			}
			return 0;

			case eck::NM_TL_BEGINDRAG:
			{
				auto p = (eck::NMTLDRAG*)lParam;
				EckDbgPrintFmt(L"Pre Drag, bRBtn = %d", p->bRBtn);
			}
			return 0;
			case eck::NM_TL_ENDDRAG:
			{
				auto p = (eck::NMTLDRAG*)lParam;
				EckDbgPrintFmt(L"Post Drag, bRBtn = %d", p->bRBtn);
			}
			return 0;
			}
		}
	}
	break;
	case WM_SIZE:
	{
		m_Lyt.Arrange(LOWORD(lParam), HIWORD(lParam));
	}
	break;
	case WM_CREATE:
		return HANDLE_WM_CREATE(hWnd, wParam, lParam, OnCreate);
	case WM_COMMAND:
	{
		switch (HIWORD(wParam))
		{
		case BN_CLICKED:
			if (m_BTRefresh.HWnd == (HWND)lParam)
			{

			}
			return 0;
		}
	}
	break;
	case WM_DESTROY:
		ClearRes();
		PostQuitMessage(0);
		return 0;
	case WM_SETTINGCHANGE:
	{
		if (eck::IsColorSchemeChangeMessage(lParam))
		{
			const auto ptc = eck::GetThreadCtx();
			ptc->UpdateDefColor();
			ptc->SetNcDarkModeForAllTopWnd(ShouldAppsUseDarkMode());
			ptc->SendThemeChangedToAllTopWindow();
			return 0;
		}
	}
	break;
	case WM_DPICHANGED:
		eck::MsgOnDpiChanged(hWnd, lParam);
		UpdateDpi(HIWORD(wParam));
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