#pragma once
#include "CApp.h"

struct CTestElem : public eck::Dui::CElem
{
	D2D1_POINT_2F pos{};
	ID2D1SolidColorBrush* pbr{};
	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_PAINT:
		{
			Dui::ELEMPAINTSTRU eps;
			BeginPaint(eps, wParam, lParam);
			pbr->SetColor(D2D1::ColorF(D2D1::ColorF::Green));
			m_pDC->FillRectangle(eps.rcfClipInElem, pbr);
			pbr->SetColor(D2D1::ColorF(D2D1::ColorF::Red));
			D2D1_ELLIPSE ellipse{};
			ellipse.point = pos;
			ellipse.radiusX = 10;
			ellipse.radiusY = 10;
			m_pDC->FillEllipse(ellipse, pbr);
			EndPaint(eps);
		}
		return 0;

		case WM_MOUSEMOVE:
		{
			pos = ECK_GET_PT_LPARAM_F(lParam);
			ClientToElem(pos);
			InvalidateRect();
		}
		break;

		case WM_CREATE:
		{
			m_pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &pbr);
		}
		break;
		}
		return __super::OnEvent(uMsg, wParam, lParam);
	}
};

class CWndMain : public Dui::CDuiWnd
{
private:
	eck::Dui::CLabel m_Label{};
	eck::Dui::CLabel m_Label2{};
	eck::Dui::CLabel m_Label3{};
	eck::Dui::CButton m_Btn{};
	eck::Dui::CList m_List{};
	eck::Dui::CTrackBar m_TB{};
	eck::Dui::CCircleButton m_CBtn{};
	eck::Dui::CScrollBar m_SB{};
	eck::Dui::CTitleBar m_TitleBar{};
	enum
	{
		cximg = 160,
		cyimg = 160,
	};
	eck::CD2dImageList m_il{ cximg, cyimg };
	eck::CEasingCurve m_ec{};

	struct ITEM
	{
		eck::CRefStrW rsFile{};
		eck::CRefStrW rs{};
		int idxImg{ 0 };
	};
	std::vector<ITEM> m_vItem{};
	eck::CSrwLock m_srw{};
	enum
	{
		WM_UIUIUI = 114514
	};


	Dui::CEdit m_EDUserName{};
	Dui::CEdit m_EDPassword{};
	Dui::CButton m_BTLogin{};
	eck::CLinearLayoutV m_Layout{};

	IDWriteTextFormat* m_pTextFormat{};

	int m_iDpi{ USER_DEFAULT_SCREEN_DPI };
	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(cxED, 300)
		ECK_DS_ENTRY(cxBT, 70)
		ECK_DS_ENTRY(cy, 30)
		ECK_DS_ENTRY(Padding, 8)
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

public:
	ECK_CWND_SINGLEOWNER(CWndMain);
	ECK_CWND_CREATE_CLS_HINST(eck::WCN_DUMMY, eck::g_hInstance);

	~CWndMain();

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	LRESULT OnElemEvent(eck::Dui::CElem* pElem, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};