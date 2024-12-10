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

BOOL CWndMain::OnCreate(HWND hWnd, CREATESTRUCT* pcs)
{
	using namespace eck::Literals;
	eck::GetThreadCtx()->UpdateDefColor();
	//eck::GetThreadCtx()->bEnableDarkModeHook = 0;
	UpdateDpiInit(eck::GetDpi(hWnd));
	m_hFont = eck::EzFont(L"微软雅黑", 9);

	eck::CMenu m1
	{
		{ L"新建(&N)" },
		{ L"打开(&O)" },
		{ L"保存(&S)" },
		{ L"另存为(&A)" },
	};

	m_Menu.Create();
	m_Menu.AppendItem(L"文件(&F)", m1.Detach());
	m_Menu.AppendItem(L"编辑(&E)", 2);
	m_Menu.AppendItem(L"视图(&V)", 3);
	m_Menu.AppendItem(L"帮助(&H)", 4);
	//SetMenu(hWnd, m_Menu.GetHMenu());

	WIN32_FIND_DATAW wfd{};
	m_hIL = ImageList_Create(90, 90, ILC_COLOR32 | ILC_ORIGINALSIZE, 20, 20);
	m_hILSmall = ImageList_Create(40, 40, ILC_COLOR32 | ILC_ORIGINALSIZE, 20, 20);
	auto path = LR"(D:\TestPic\)"_rs;
	const auto hFind = FindFirstFileW((path + LR"(*.png)").Data(), &wfd);
	int i{};
	do
	{
		IWICBitmapDecoder* pd;
		eck::CreateWicBitmapDecoder(
			(path + wfd.cFileName).Data(), pd);
		m_vItem.emplace_back(wfd.cFileName, i);

		IWICBitmap* pb;
		//eck::CreateWicBitmap(pb, pd, 90, 90, eck::DefWicPixelFormat, WICBitmapInterpolationModeHighQualityCubic);
		eck::CreateWicBitmap(pb, pd);
		//eck::SaveWicBitmap(eck::Format(LR"(D:\TestPic\%s.png)", wfd.cFileName).Data(), pb);
		const auto h = eck::CreateHICON(pb);
		ImageList_AddIcon(m_hIL, h);

		eck::CreateWicBitmap(pb, pd, 40, 40,
			eck::DefWicPixelFormat, WICBitmapInterpolationModeHighQualityCubic);
		const auto hSmall = eck::CreateHICON(pb);
		ImageList_AddIcon(m_hILSmall, hSmall);
		/*if (i == 41)
			break;*/
		++i;
	} while (FindNextFileW(hFind, &wfd));
	FindClose(hFind);


	{
		{
			m_BTTest.Create(L"Test Button", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0,
				0, 0, m_Ds.cxTestBtn, m_Ds.cyTestBtn, hWnd, 0);
			m_LytCtrl.Add(&m_BTTest, {}, eck::LF_FIX);
		}
		m_LytMain.Add(&m_LytCtrl, {}, eck::LF_FIX);

		m_Tab.Create(nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, 0,
			0, 0, 10, 10, hWnd, 0);
		m_LytMain.Add(&m_Tab, {}, eck::LF_FILL, 1);
	}
	m_Tab.InsertItem(L"Button");
	m_Tab.InsertItem(L"ComboBox");
	m_Tab.InsertItem(L"Dialog");
	m_Tab.InsertItem(L"Label");
	m_Tab.InsertItem(L"ListBox");
	m_Lyt.LoInitDpi(m_iDpi);

	const MARGINS Mar{ .cyTopHeight = m_Ds.Margin };
	// Button
	{
		const auto pl = new eck::CFlowLayoutV{};
		m_vLyt.push_back(pl);
		m_BTPush.Create(L"Push Button", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0,
			0, 0, m_Ds.cxBtn, m_Ds.cyBtn, m_Tab.HWnd, 0);
		/*AllowDarkModeForWindow(m_BTPush.HWnd, TRUE);
		m_BTPush.SetExplorerTheme();*/
		pl->Add(&m_BTPush, Mar, eck::LF_FIX);

		m_BTRadio.Create(L"Radio Button", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 0,
			0, 0, m_Ds.cxBtn, m_Ds.cyBtn, m_Tab.HWnd, 0);
		pl->Add(&m_BTRadio, Mar, eck::LF_FIX);

		m_BTCheck.Create(L"Check Button", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 0,
			0, 0, m_Ds.cxBtn, m_Ds.cyBtn, m_Tab.HWnd, 0);
		pl->Add(&m_BTCheck, Mar, eck::LF_FIX);

		m_BTGroup.Create(L"Group Button", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 0,
			0, 0, m_Ds.cxBtn, m_Ds.cyBtn, m_Tab.HWnd, 0);
		pl->Add(&m_BTGroup, Mar, eck::LF_FIX);

		m_BT3State.Create(L"3-State Button", WS_CHILD | WS_VISIBLE | BS_AUTO3STATE, 0,
			0, 0, m_Ds.cxBtn, m_Ds.cyBtn, m_Tab.HWnd, 0);
		pl->Add(&m_BT3State, Mar, eck::LF_FIX);

		m_BTCmdLink.Create(L"Command Link Button", WS_CHILD | WS_VISIBLE | BS_COMMANDLINK, 0,
			0, 0, m_Ds.cxBtn * 2, m_Ds.cyBtn, m_Tab.HWnd, 0);
		m_BTCmdLink.SetNote(L"Note Text");
		pl->Add(&m_BTCmdLink, Mar, eck::LF_FIX);

		m_Lyt.Add(pl, {}, eck::LF_FILL);
	}

	// ComboBox
	{
		const auto pl = new eck::CFlowLayoutV{};
		m_vLyt.push_back(pl);
		m_CBBSimple.Create(nullptr, WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_SIMPLE, 0,
			0, 0, m_Ds.cxCombo, m_Ds.cyCombo * 4, m_Tab.HWnd, 0);
		pl->Add(&m_CBBSimple, Mar, eck::LF_FIX);
		EckCounter(50, i)
			m_CBBSimple.InsertString((eck::ToStr(i) + L" Item").Data());

		m_CBBDropDown.Create(nullptr, WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWN, 0,
			0, 0, m_Ds.cxCombo, m_Ds.cyCombo, m_Tab.HWnd, 0);
		pl->Add(&m_CBBDropDown, Mar, eck::LF_FIX);
		EckCounter(20, i)
			m_CBBDropDown.InsertString((eck::ToStr(i) + L" Item").Data());

		m_CBBDropDownList.Create(nullptr, WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST, 0,
			0, 0, m_Ds.cxCombo, m_Ds.cyCombo, m_Tab.HWnd, 0);
		pl->Add(&m_CBBDropDownList, Mar, eck::LF_FIX);
		EckCounter(10, i)
			m_CBBDropDownList.InsertString((eck::ToStr(i) + L" Item").Data());

		m_CBE.Create(nullptr, WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST, 0,
			0, 0, m_Ds.cxCombo, m_Ds.cyCombo * 6, m_Tab.HWnd, 0);
		m_CBE.SetImageList(m_hILSmall);
		for (const auto& e : m_vItem)
			m_CBE.InsertItem(e.rsText.Data(), -1, e.idxImage);
		pl->Add(&m_CBE, Mar, eck::LF_FIX);

		m_CBN.Create(nullptr, WS_CHILD | WS_VISIBLE, 0,
			0, 0, m_Ds.cxCombo, m_Ds.cyCombo, m_Tab.HWnd, 0);
		m_CBN.SetView(eck::CComboBoxNew::View::DropDownEdit);
		pl->Add(&m_CBN, Mar, eck::LF_FIX);
		//m_CBN.GetListBox().SetItemCount(8);
		m_CBN.GetListBox().SetItemCount((int)m_vItem.size());
		m_Lyt.Add(pl, {}, eck::LF_FILL);
	}

	// Dialog
	{
		const auto pl = new eck::CFlowLayoutV{}; 
		m_vLyt.push_back(pl);
		m_BTInputBox.Create(L"Test InputBox", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0,
			0, 0, m_Ds.cxBtn, m_Ds.cyBtn, m_Tab.HWnd, 0);
		pl->Add(&m_BTInputBox, {}, eck::LF_FIX);

		m_BTTaskDialog.Create(L"Test TaskDialog", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0,
			0, 0, m_Ds.cxBtn, m_Ds.cyBtn, m_Tab.HWnd, 0);
		pl->Add(&m_BTTaskDialog, {}, eck::LF_FIX);
		m_Lyt.Add(pl, {}, eck::LF_FILL);
	}

	// Label
	{
		const auto pl = new eck::CFlowLayoutV{};
		m_vLyt.push_back(pl);
		m_LA.Create(L"This is a Label\r\nWith multiple lines.",
			WS_CHILD | WS_VISIBLE | WS_BORDER, 0,
			0, 0, m_Ds.cxBtn * 2, m_Ds.cyBtn * 2, m_Tab.HWnd, 0);
		m_LA.SetAlign(TRUE, eck::Align::Center);
		m_LA.SetAlign(FALSE, eck::Align::Center);
		m_LA.SetAutoWrap(TRUE);
		//m_LA.SetTransparent(TRUE);
		pl->Add(&m_LA, Mar, eck::LF_FIX);

		m_Lyt.Add(pl, {}, eck::LF_FILL);
	}

	// ListBox
	{
		const auto pl = new eck::CFlowLayoutV{};
		m_vLyt.push_back(pl);
		m_LBN.Create(nullptr,WS_CHILD | WS_VISIBLE | WS_VSCROLL, 0,
			0, 0, m_Ds.cxLB, m_Ds.cyLB, m_Tab.HWnd, 0);
		//m_LBN.SetItemCount((int)m_vItem.size());
		m_LBN.SetItemCount(100);
		m_LBN.SetMultiSel(TRUE);
		m_LBN.SetExtendSel(TRUE);
		pl->Add(&m_LBN, Mar, eck::LF_FIX);

		m_Lyt.Add(pl, {}, eck::LF_FILL);
	}

	m_Tab.SetCurSel(2);
	m_Lyt.ShowFrame(2);

	m_Tab.GetSignal().Connect([this](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)->LRESULT
		{
			switch (uMsg)
			{
			case WM_CTLCOLOR:
			case WM_CTLCOLORBTN:
			case WM_CTLCOLORDLG:
			case WM_CTLCOLOREDIT:
			case WM_CTLCOLORLISTBOX:
			case WM_CTLCOLORMSGBOX:
			case WM_CTLCOLORSCROLLBAR:
			case WM_CTLCOLORSTATIC:
			{
				bHandled = TRUE;
				return CWndMain::OnMsg(hWnd, uMsg, wParam, lParam);
			}
			break;

			case WM_COMMAND:
			{
				if (HIWORD(wParam) == BN_CLICKED)
					if (m_BTCmdLink.HWnd == (HWND)lParam)
					{
						constexpr PCWSTR Note[]
						{
							L"Note Text",
							L"More Note Text",
							L"Even More Note Text",
							L"And Even More Note Text",
							L"Okay, I'm done",
						};
						static int i = 0;
						m_BTCmdLink.SetNote(Note[i++ % ARRAYSIZE(Note)]);
					}
					else if (m_BTInputBox.HWnd == (HWND)lParam)
					{
						eck::INPUTBOXOPT Opt{};
						Opt.pszTitle = L"InputBox Test";
						Opt.pszMainTip = L"Please input something";
						Opt.pszTip = L"This topic discusses the common controls, "
							L"a set of windows that are implemented by the common "
							L"control library, Comctl32.dll, which is a DLL included "
							L"with the Windows operating system.Like other control "
							L"windows, a common control is a child window that an "
							L"application uses in conjunction with another window to "
							L"enable interaction with the user.";
						Opt.pszInitContent = L"This is the initial content";
						Opt.uFlags = eck::IPBF_CENTERPARENT |
							eck::IPBF_FIXWIDTH | eck::IPBF_MULTILINE;
						eck::CInputBox ib{};
						//ib.DlgBox(HWnd, &Opt);
						InputBox(Opt.rsInput, HWnd, Opt.pszMainTip, Opt.pszTip, 
							Opt.pszInitContent, Opt.pszTitle, FALSE);
					}
					else if (m_BTTaskDialog.HWnd == (HWND)lParam)
					{
						TestTaskDialog();
					}
			}
			break;

			case WM_NOTIFY:
			{
				const auto pnm = (NMHDR*)lParam;
				if (pnm->hwndFrom == m_CBN.HWnd)
				{
					bHandled = TRUE;
					switch (pnm->code)
					{
					case eck::NM_LBN_GETDISPINFO:
					{
						const auto p = (eck::NMLBNGETDISPINFO*)lParam;
						const auto idx = p->Item.idxItem % m_vItem.size();
						p->Item.pszText = m_vItem[idx].rsText.Data();
						p->Item.cchText = (int)m_vItem[idx].rsText.Size();
					}
					return TRUE;
					}
				}
				else if (pnm->hwndFrom == m_LBN.HWnd)
				{
					bHandled = TRUE;
					switch (pnm->code)
					{
					case eck::NM_LBN_GETDISPINFO:
					{
						const auto p = (eck::NMLBNGETDISPINFO*)lParam;
						const auto idx = p->Item.idxItem % m_vItem.size();
						p->Item.pszText = m_vItem[idx].rsText.Data();
						p->Item.cchText = (int)m_vItem[idx].rsText.Size();
					}
					return TRUE;
					}
				}
			}
			break;
			}
			return 0;
		});

	eck::SetFontForWndAndCtrl(hWnd, m_hFont);
	return TRUE;
}

void CWndMain::TestButton()
{
	EckDbgPrint(L"=====Test Button=====");
	m_BTPush.ReCreate();
	EckDbgPrint(L"Push Button re-created.");

	m_BTRadio.ReCreate();
	EckDbgPrint(L"Radio Button re-created.");

	m_BTCheck.ReCreate();
	EckDbgPrint(L"Check Button re-created.");

	m_BTGroup.ReCreate();
	EckDbgPrint(L"Group Button re-created.");

	m_BT3State.ReCreate();
	EckDbgPrint(L"3-State Button re-created.");

	m_BTCmdLink.ReCreate();
	EckDbgPrint(L"Command Link Button re-created.");
	EckDbgPrint(L"=====Test Button End=====");
}

void CWndMain::TestComboBox()
{
	EckDbgPrint(L"=====Test ComboBox=====");
	m_CBBSimple.ReCreate();
	EckDbgPrint(L"Simple ComboBox re-created.");

	m_CBBDropDown.ReCreate();
	EckDbgPrint(L"DropDown ComboBox re-created.");

	m_CBBDropDownList.ReCreate();
	EckDbgPrint(L"DropDownList ComboBox re-created.");

	m_CBE.ReCreate();
	EckDbgPrint(L"Editable ComboBox re-created.");
	EckDbgPrint(L"=====Test ComboBox End=====");

	((eck::CLayoutBase*)m_vLyt[1])->Refresh();
}

void CWndMain::TestTaskDialog()
{
	TASKDIALOGCONFIG page1{};
	// PAGE 1
	const TASKDIALOG_BUTTON radio[] = { {10, L"单选按钮1"},
										{11, L"RadioButton2"},
										{12, L"单选按钮3"} };

	const TASKDIALOG_BUTTON commands[] = { {10, L"正常"},
											{11, L"禁用"},
											{12, L"UAC"},
											{13, L"UAC + 禁用"},
											{14, L"正常"} };
	page1.cbSize = sizeof(TASKDIALOGCONFIG);
	page1.dwFlags = TDF_ENABLE_HYPERLINKS | TDF_USE_COMMAND_LINKS | TDF_EXPAND_FOOTER_AREA | TDF_SHOW_MARQUEE_PROGRESS_BAR;
	page1.dwCommonButtons = TDCBF_CLOSE_BUTTON;
	page1.pszWindowTitle = L"任务对话框标题测试";
	page1.pszMainIcon = TD_INFORMATION_ICON;
	page1.pszMainInstruction = L"任务对话框测试";
	page1.pszContent = LR"(VirtualAlloc 无法保留保留页。 它可以提交已提交的页面。 这意味着你可以提交一系列页面，无论它们是否已提交，并且函数不会失败。

可以使用 VirtualAlloc 保留页面块，然后对 virtualAlloc 进行其他调用，以提交保留块中的单个页面。 这样一个进程就可以保留其虚拟地址空间的范围，而无需消耗物理存储，直到需要它。

如果 lpAddress 参数未 NULL，则该函数将使用 lpAddress，dwSize 参数来计算要分配的页面区域。 整个页面范围的当前状态必须与 flAllocationType 参数指定的分配类型兼容。 否则，该函数将失败，并且未分配任何页面。 如前所述，此兼容性要求不排除提交已提交的页面。)";
	page1.cButtons = 5;
	page1.pButtons = commands;
	page1.nDefaultButton = TDCBF_CLOSE_BUTTON;
	page1.cRadioButtons = 3;
	page1.pRadioButtons = radio;
	page1.pszVerificationText = L"我是复选框标题";
	page1.pszExpandedInformation = LR"(展开测试：
<a>VirtualFree</a>函数可以取消提交页面、释放页面的存储，也可以同时取消提交和释放已提交的页面。 它还可以释放保留页，使其成为免费页面。)";
	page1.pszExpandedControlText = L"折叠";
	page1.pszCollapsedControlText = L"展开";
	page1.pszFooterIcon = TD_SHIELD_ICON;
	page1.pszFooter = LR"(最低支持的客户端	Windows XP [桌面应用 |UWP 应用]
支持的最低服务器	Windows Server 2003 [桌面应用 |UWP 应用]
目标平台	窗户)";
	//page1.pfCallback = taskdialogcallback;
	page1.lpCallbackData = 1;
	eck::TASKDIALOGCTX Ctx{};
	Ctx.ptdc = &page1;
	eck::CTaskDialog td{};
	const auto hSlot =td.GetCallbackSignal().Connect(
		[&](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)->HRESULT
		{
			td.SetShieldIcon(12, TRUE);
			td.SetShieldIcon(13, TRUE);
			return S_OK;
		});
	td.DlgBox(HWnd, &Ctx);
	td.GetCallbackSignal().Disconnect(hSlot);
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
		m_LytMain.Arrange(LOWORD(lParam), HIWORD(lParam));

		RECT rc;
		GetClientRect(m_Tab.HWnd, &rc);
		m_Tab.AdjustRect(&rc, FALSE);
		m_Lyt.Arrange(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
	}
	break;

	case WM_NOTIFY:
	{
		const auto pnm = (NMHDR*)lParam;
		if (pnm->hwndFrom == m_Tab.HWnd)
			switch (pnm->code)
			{
			case TCN_SELCHANGE:
				m_Lyt.ShowFrame(m_Tab.GetCurSel());
				break;
			}
	}
	break;

	case WM_CREATE:
		return HANDLE_WM_CREATE(hWnd, wParam, lParam, OnCreate);
	case WM_COMMAND:
	{
		switch (HIWORD(wParam))
		{
		case BN_CLICKED:
			if (m_BTTest.HWnd == (HWND)lParam)
				switch (m_Tab.GetCurSel())
				{
				case 0:
					TestButton();
					break;
				case 1:
					TestComboBox();
					break;
				case 2:
					break;
				case 3:
					break;
				case 4:
					break;
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
	IntCreate(dwExStyle, eck::WCN_DUMMY, pszText, dwStyle,
		x, y, cx, cy, hParent, hMenu, eck::g_hInstance, NULL);
	return NULL;
}