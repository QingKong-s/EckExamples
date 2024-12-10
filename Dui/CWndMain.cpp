#include "pch.h"

#include "CWndMain.h"

D2D1_MATRIX_4X4_F TMat{};
D2D1_MATRIX_4X4_F TMat2{};

void CWndMain::UpdateDpi(int iDpi)
{
	const int iDpiOld = m_iDpi;
	UpdateDpiInit(iDpi);
	m_Layout.LoOnDpiChanged(iDpi);

	auto pTf = eck::CreateDefTextFormat(iDpi);
	std::swap(m_pTextFormat, pTf);
	pTf->Release();

	UpdateFixedUISize();
}

void CWndMain::UpdateFixedUISize()
{
}

void CWndMain::ClearRes()
{
	eck::SafeRelease(m_pTextFormat);
}

BOOL CWndMain::OnCreate(HWND hWnd, CREATESTRUCT* pcs)
{
	eck::GetThreadCtx()->UpdateDefColor();

	UpdateDpiInit(eck::GetDpi(hWnd));
	m_pTextFormat = eck::CreateDefTextFormat(m_iDpi);

	/*MARGINS Mar{ .cyBottomHeight = m_Ds.Padding };
	{
		m_EDUserName.Create(L"用户名", Dui::DES_VISIBLE, 0,
			0, 0, m_Ds.cxED, m_Ds.cy, nullptr, this);
		m_EDUserName.SetTextFormat(m_pTextFormat);
		m_Layout.Add(&m_EDUserName, Mar, eck::LF_FIX | eck::LF_ALIGN_CENTER);

		m_EDPassword.Create(L"密码", Dui::DES_VISIBLE, 0,
			0, 0, m_Ds.cxED, m_Ds.cy, nullptr, this);
		m_EDPassword.SetTextFormat(m_pTextFormat);
		m_Layout.Add(&m_EDPassword, Mar, eck::LF_FIX | eck::LF_ALIGN_CENTER);

		auto pTf = eck::CreateDefTextFormat(m_iDpi);

		m_BTLogin.Create(L"登录", Dui::DES_VISIBLE, 0,
			0, 0, m_Ds.cxBT, m_Ds.cy, nullptr, this);
		pTf->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		m_BTLogin.SetTextFormat(pTf);
		pTf->Release();
		m_Layout.Add(&m_BTLogin, Mar, eck::LF_FIX | eck::LF_ALIGN_CENTER);
	}
	m_Layout.LoInitDpi(m_iDpi);*/

	using namespace eck;
	using namespace eck::Dui;
	using namespace DirectX;
	ID2D1Bitmap* pBitmap;
	auto pTfDef = CreateDefTextFormat(m_iDpi);

	eck::LoadD2dBitmap(LR"(E:\Desktop\Temp\111.bmp)", GetD2D().GetDC(), pBitmap);

		//auto sizez = pBitmap->GetSize();
	m_il.BindRenderTarget(GetD2D().GetDC());
	//SetBkgBitmap(pBitmap);

	//m_Label3.Create(L"测试标签😍😍", eck::Dui::DES_VISIBLE | eck::Dui::DES_BLURBKG, 0,
	//	0, 0, 800, 900, NULL, this, NULL);
	WIN32_FIND_DATAW wfd;
	HANDLE hFind = FindFirstFileW(LR"(C:\*.*)", &wfd);
	int c = 0;
	do
	{
		m_vItem.emplace_back(LR"(C:\)"_rs + wfd.cFileName, wfd.cFileName);
		++c;
		if (c == -100)
			break;
	} while (FindNextFileW(hFind, &wfd));
	FindClose(hFind);

	eck::LoadD2dBitmap(LR"(D:\@重要文件\@我的工程\PlayerNew\Res\DefCover.png)", GetD2D().GetDC(),
		pBitmap, cximg, cyimg);
	m_il.AddImage(pBitmap);

	std::thread t([this]
		{
			return;
			for (int i{}; auto & e : m_vItem)
			{
				ID2D1Bitmap* p;
				eck::LoadD2dBitmap(e.rsFile.Data(), GetD2D().GetDC(),
					p, cximg, cyimg);
				const int idx = m_il.AddImage(p);
				p->Release();
				m_srw.EnterWrite();
				e.idxImg = idx;
				m_srw.LeaveWrite();
				PostMsg(WM_UIUIUI, i, 0);
				++i;
			}
		});
	t.detach();

	eck::Dui::CElem* pElem = nullptr; &m_Label3;
	pElem = 0;
	Dui::CElem* pTestElem = new CTestElem{};
	pTestElem = &m_List;
	pTestElem->GetSignal().Connect([=, this](UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bProcessed)->LRESULT
		{
			if (uMsg == EWM_COMPOSITE)
			{
				bProcessed = TRUE;
				auto p = (COMP_INFO*)wParam;
				if (p->pElem != &m_List)
				{
					auto mat = XMLoadFloat4x4((XMFLOAT4X4*)&TMat);
					auto rcNew = *p->prc;
					p->pElem->ElemToClient(rcNew);
					pTestElem->ClientToElem(rcNew);

					// 映射到目标矩形
					D2D1_POINT_2F ptTo[4];
					RectCornerToPoint(rcNew, ptTo);

					for (auto& e : ptTo)
					{
						XMFLOAT4 vp1{ e.x,e.y,0.f,1.f };
						auto vp = XMVector4Transform(XMLoadFloat4(&vp1), mat);
						float w = XMVectorGetW(vp);
						vp = XMVectorDivide(vp, XMVectorSet(w, w, w, w));
						e.x = XMVectorGetX(vp);
						e.y = XMVectorGetY(vp);

						pTestElem->ElemToClient(e);
						p->pElem->ClientToElem(e);
					}

					D2D1_MATRIX_4X4_F matNew;
					auto rcl = p->pElem->GetRectF();
					rcl.right = rcl.right - rcl.left;
					rcl.bottom = rcl.bottom - rcl.top;
					rcl.left = 0;
					rcl.top = 0;
					CalcDistortMatrix(rcl, ptTo, matNew);

					pTestElem->GetDC()->DrawBitmap(GetCacheBitmap(), nullptr,
						1.f, D2D1_INTERPOLATION_MODE_MULTI_SAMPLE_LINEAR, p->prc, &matNew);
				}
				else
				{
					pTestElem->GetDC()->DrawBitmap(GetCacheBitmap(), nullptr,
						1.f, D2D1_INTERPOLATION_MODE_MULTI_SAMPLE_LINEAR, p->prc, &TMat);
				}
				return 0;
			}
			else if (uMsg == EWM_COMP_POS)
			{
				bProcessed = TRUE;
				auto p = (COMP_POS*)wParam;
				if (p->pElem == pTestElem)
					CptTransformPointNonAffine(TMat, TMat2, p);
				else
				{
					p->pElem->ElemToClient(p->pt);
					pTestElem->ClientToElem(p->pt);
					CptTransformPointNonAffine(TMat, TMat2, p);
					pTestElem->ElemToClient(p->pt);
					p->pElem->ClientToElem(p->pt);
				}
			}
			else if (uMsg == WM_SIZE)
			{
				pTestElem->OnEvent(uMsg, wParam, lParam);
				bProcessed = TRUE;
				auto rc{ pTestElem->GetViewRect() };
				rc.top -= 30;
				rc.right += 30;
				rc.bottom += 150;
				pTestElem->SetPostCompositedRect(rc);
			}
			return 0;
		}, 1);
	pTestElem->Create(nullptr, DES_VISIBLE | DES_TRANSPARENT /*| DES_COMPOSITED*/, 0,
		50, 50, 650, 450, pElem, this);// 50  70  650  670

	D2D1_RECT_F rcl{};// = m_List.GetFirstChildElem()->GetRectF();
	rcl.right = rcl.right - rcl.left;
	rcl.bottom = rcl.bottom - rcl.top;
	rcl.left = 0;
	rcl.top = 0;
	D2D1_POINT_2F pt[4]
	{
		{ rcl.left,rcl.top },
		{ rcl.right,rcl.top + 200 },
		{ rcl.left,rcl.bottom },
		{ rcl.right,rcl.bottom + 200 },
	};

	CalcDistortMatrix(rcl, pt, TMat2);

	rcl = pTestElem->GetRectF();
	rcl.right = rcl.right - rcl.left;
	rcl.bottom = rcl.bottom - rcl.top;
	rcl.left = 0;
	rcl.top = 0;
	D2D1_POINT_2F pt2[4]
	{
		{ rcl.left,rcl.top },
		{ rcl.right,rcl.top - 30 },
		{ rcl.left,rcl.bottom },
		{ rcl.right + 30,rcl.bottom + 150 },
	};

	CalcDistortMatrix(rcl, pt2, TMat);
	CalcInverseDistortMatrix(rcl, pt2, TMat2);

	XMFLOAT4 vp1{ pt2[1].x,pt2[1].y,0.f,1.f};
	auto mat = XMLoadFloat4x4((XMFLOAT4X4*)&TMat2);
	auto vp = XMVector4Transform(XMLoadFloat4(&vp1), mat);
	float w = XMVectorGetW(vp);
	vp = XMVectorDivide(vp, XMVectorSet(w, w, w, w));
	m_List.SetTextFormat(pTfDef);
	m_List.SetItemHeight(240);
	m_List.SetItemWidth(200);
	//m_List.SetImageSize(-1);
	m_List.SetItemPadding(5);
	m_List.SetItemPaddingH(5);
	m_List.SetItemCount((int)m_vItem.size());

	m_TitleBar.Create(nullptr, eck::Dui::DES_VISIBLE | eck::Dui::DES_TRANSPARENT, 0,
		0, 0, 800, 50, nullptr, this);

	m_Btn.Create(L"144", eck::Dui::DES_VISIBLE | eck::Dui::DES_TRANSPARENT, 0,
		600, 300, 300, 70, NULL, this, NULL);
	m_Btn.SetTextFormat(pTfDef);

	const auto pBTDark = new Dui::CButton{};
	pBTDark->Create(L"Dark", eck::Dui::DES_VISIBLE | eck::Dui::DES_TRANSPARENT, 0,
		80, 400, 100, 70, NULL, this, 114514);
	pBTDark->SetTextFormat(pTfDef);

	m_CBtn.Create(NULL, eck::Dui::DES_VISIBLE | eck::Dui::DES_TRANSPARENT, 0,
		150, 200, 60, 60, NULL, this, NULL);

	eck::LoadD2dBitmap(LR"(D:\@重要文件\@我的工程\PlayerNew\Res\Speed.png)",
		GetD2D().GetDC(), pBitmap, 40, 40);
	m_CBtn.SetImage(pBitmap);
	pBitmap->Release();

	const auto pTB = new Dui::CTrackBar{};
	pTB->Create(NULL, eck::Dui::DES_VISIBLE | eck::Dui::DES_TRANSPARENT, 0,
		100, 500, 300, 50, NULL, this, NULL);
	pTB->SetRange(0, 100);
	pTB->SetPos(50);
	pTB->SetTrackSize(20);

	/*m_vItem.resize(100);
	EckCounter(m_vItem.size(), i)
	{
		m_il.AddImage(pBitmap);
		m_vItem[i].rs = eck::ToStr(i);
		m_vItem[i].pBitmap = pBitmap;
	}
	m_List.SetItemCount((int)m_vItem.size());*/
	m_List.SetImageList(&m_il);
	///*m_List.SetInsertMark(5);
	//m_List.SetTopExtraSpace(100);
	//m_List.SetBottomExtraSpace(100);*/

	/*m_Label2.Create(L"测试标签😍😍", eck::Dui::DES_VISIBLE | eck::Dui::DES_BLURBKG | 0, 0,
		0, 0, 600, 100, &m_List, this, NULL);
	m_Label2.SetTextFormat(GetDefTextFormat());

	m_Label.Create(L"我是标签",
		eck::Dui::DES_VISIBLE | eck::Dui::DES_BLURBKG, 0,
		0, 500, 600, 100, &m_List, this, NULL);
	m_Label.SetTextFormat(GetDefTextFormat());

	
	ID2D1Bitmap* pBmp;
	eck::LoadD2dBitmap(LR"(D:\@重要文件\@我的工程\PlayerNew\Res\Tempo.png)",
		GetD2D().GetDC(), pBmp, 54, 54);
	m_Btn.SetImage(pBmp);
	pBmp->Release();

	m_TB.Create(NULL, eck::Dui::DES_VISIBLE | eck::Dui::DES_TRANSPARENT, 0,
		100, 0, 700, 300, NULL, this, NULL);
	m_TB.SetRange(0, 100);
	m_TB.SetPos(50);
	m_TB.SetTrackSize(20);*/
	//m_TB.SetVertical(true);

	//if (pBitmap)
	//	pBitmap->Release();

	//m_SB.Create(NULL, eck::Dui::DES_VISIBLE | eck::Dui::DES_TRANSPARENT, 0,
	//	400, 300, GetDs().CommSBCxy, 300, NULL, this);
	//const auto psv = m_SB.GetScrollView();
	//psv->SetRange(-100, 100);
	//psv->SetPage(40);

	
	auto psz =
		L"应用程序😱😱😭😅😡😎调用此函数以获取一组对应于一系列文本位置的命中测试指标。主要用法之一是实现文本字符串的突出显示选择。\n"
		L"当 hitTestMetrics 的缓冲区大小太小，无法容纳函数计算的所有区域时，函数返回E_NOT_SUFFICIENT_BUFFER，这等效于 HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) 。"
		L"在这种情况下，函数将输出值 * actualHitTestMetricsCount 设置为计算的几何图形数。\n"
		L"应用程序负责分配更大的新缓冲区，并再次调用函数。"
		L"应用程序😱😱😭😅😡😎调用此函数以获取一组对应于一系列文本位置的命中测试指标。主要用法之一是实现文本字符串的突出显示选择。\n"
		L"当 hitTestMetrics 的缓冲区大小太小，无法容纳函数计算的所有区域时，函数返回E_NOT_SUFFICIENT_BUFFER，这等效于 HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) 。"
		L"在这种情况下，函数将输出值 * actualHitTestMetricsCount 设置为计算的几何图形数。\n"
		L"应用程序负责分配更大的新缓冲区，并再次调用函数。";
	auto ped = new eck::Dui::CEdit{};
	ped->Create(psz, eck::Dui::DES_VISIBLE | eck::Dui::DES_TRANSPARENT, 0,
		100, 150, 600, 400, nullptr, this);

	EnableDragDrop(TRUE);

	/*m_ec.SetWnd(HWnd);
	m_ec.SetParam((LPARAM)this);
	m_ec.SetCallBack([](float fCurrValue, float fOldValue, LPARAM lParam)
		{
			auto p = (CTestDui*)lParam;
			p->m_List.SetPos((int)fCurrValue, 70);
			p->Redraw();
		});*/
	Redraw();

	return TRUE;
}

CWndMain::~CWndMain()
{
}

LRESULT CWndMain::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	using namespace eck;
	using namespace eck::Dui;
	using namespace DirectX;
	
	LRESULT lr;
	if (DwmDefWindowProc(hWnd, uMsg, wParam, lParam, &lr))
		return lr;

	switch (uMsg)
	{
	case eck::Dui::WM_DRAGENTER:
	case eck::Dui::WM_DRAGOVER:
		return TRUE;
		{
			auto p = (eck::Dui::DRAGDROPINFO*)wParam;
			*(p->pdwEffect) = DROPEFFECT_COPY;
		}
	case eck::Dui::WM_DRAGLEAVE:
		return TRUE;
	case eck::Dui::WM_DROP:
		return TRUE;
		{
			auto p = (eck::Dui::DRAGDROPINFO*)wParam;
			FORMATETC fe = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
			STGMEDIUM sm{ TYMED_HGLOBAL };
			IWICBitmapDecoder* pDecoder{};
			IWICBitmap* pWicBmp{};
			ID2D1Bitmap* pBitmap{};
			if (p->pDataObj->GetData(&fe, &sm) == S_OK)
			{
				HDROP hDrop = (HDROP)sm.hGlobal;
				UINT uFileCount = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);
				for (UINT i = 0; i < uFileCount; ++i)
				{
					WCHAR szFile[MAX_PATH];
					DragQueryFileW(hDrop, i, szFile, ARRAYSIZE(szFile));
					int idxImg = -1;
					if (SUCCEEDED(eck::CreateWicBitmapDecoder(szFile, pDecoder)))
						if (SUCCEEDED(eck::CreateWicBitmap(pWicBmp, pDecoder, 160, 160)))
							if (SUCCEEDED(GetD2D().GetDC()->CreateBitmapFromWicBitmap(pWicBmp, &pBitmap)))
								idxImg = m_il.AddImage(pBitmap);
					//m_vItem.emplace_back(PathFindFileNameW(szFile), idxImg);

					eck::SafeRelease(pBitmap);
					eck::SafeRelease(pDecoder);
					eck::SafeRelease(pWicBmp);

					//m_Label2.SetText(mi.rsTitle);
					//m_Label2.SetPic(mi.pCoverData);
				}
				DragFinish(hDrop);
				m_List.SetItemCount((int)m_vItem.size());
			}
		}
		return TRUE;

	case WM_NCCALCSIZE:
	{
		const auto cx = GetSystemMetrics(SM_CXSIZEFRAME),
			cy = GetSystemMetrics(SM_CYSIZEFRAME);
		return eck::MsgOnNcCalcSize(wParam, lParam, MakeMarginHV(cx, cy));
	}

	case WM_SIZE:
	{
		/*MARGINS m{ .cyTopHeight = -1 };
		m.cyTopHeight = -1;
		DwmExtendFrameIntoClientArea(hWnd, &m);
		int a{ DWMSBT_TRANSIENTWINDOW };
		DwmSetWindowAttribute(hWnd, DWMWA_SYSTEMBACKDROP_TYPE, &a, sizeof(a));*/

		m_Layout.Arrange(0, eck::DpiScale(70, m_iDpi),
			LOWORD(lParam), HIWORD(lParam));

		//ACCENT_POLICY ap{};
		//ap.AccentState = ACCENT_ENABLE_BLURBEHIND;
		//ap.AccentFlags = 0;
		//ap.GradientColor = 0;
		//ap.AnimationId = 0;

		//WINDOWCOMPOSITIONATTRIBDATA data{};
		//data.Attrib = WCA_ACCENT_POLICY;
		//data.cbData = sizeof(ACCENT_POLICY);
		//data.pvData = &ap;
		//SetWindowCompositionAttribute(hWnd, &data);

		m_TitleBar.SetRect({ 0, 0,Phy2Log(LOWORD(lParam)), 100 });
	}
	break;

	case WM_CREATE:
	{
		const auto lResult = __super::OnMsg(hWnd, uMsg, wParam, lParam);
		//SetUserDpi(192);
		HANDLE_WM_CREATE(hWnd, wParam, lParam, OnCreate);
		return lResult;
	}
	break;

	case WM_UIUIUI:
	{
		m_List.RedrawItem((int)wParam);
	}
	return 0;

	case WM_DESTROY:
		__super::OnMsg(hWnd, uMsg, wParam, lParam);
		ClearRes();
		PostQuitMessage(0);
		break;

	case WM_SYSCOLORCHANGE:
		eck::MsgOnSysColorChangeMainWnd(hWnd, wParam, lParam);
		break;
	case WM_SETTINGCHANGE:
		eck::MsgOnSettingChangeMainWnd(hWnd, wParam, lParam);
		break;
	case WM_DPICHANGED:
		eck::MsgOnDpiChanged(hWnd, lParam);
		UpdateDpi(HIWORD(wParam));
		return 0;
	}

	return CDuiWnd::OnMsg(hWnd, uMsg, wParam, lParam);
}

LRESULT CWndMain::OnElemEvent(eck::Dui::CElem* pElem, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	using namespace eck::Dui;
	switch (uMsg)
	{
	case LEE_GETDISPINFO:
	{
		auto p = (LEEDISPINFO*)lParam;
		m_srw.EnterRead();
		const auto& e = m_vItem[p->idx];
		p->pszText = e.rs.Data();
		p->cchText = e.rs.Size();
		p->idxImg = e.idxImg;
		m_srw.LeaveRead();
		//p->pImage = e.pBitmap;
		//auto s = e.pBitmap->GetSize();
		//p->cxImage = s.width;
		//p->cyImage = s.height;
	}
	return 0;
	case EE_COMMAND:
	{
		if (pElem == &m_Btn)
		{
			if (pElem->GetText() == L"144")
			{
				pElem->SetText(L"96");
				SetUserDpi(96);
			}
			else
			{
				pElem->SetText(L"144");
				SetUserDpi(144);
			}
			Redraw();
		}
		else if (pElem->GetID() == 114514)
		{
			SwitchStdThemeMode(1);
		}
	}
	return 0;
	}
	return CDuiWnd::OnElemEvent(pElem, uMsg, wParam, lParam);
}