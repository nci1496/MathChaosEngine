
// MathChaosEngineView.cpp: CMathChaosEngineView 类的实现
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "MathChaosEngine.h"
#endif

#include "MathChaosEngineDoc.h"
#include "MathChaosEngineView.h"

#include <algorithm>
#include <cmath>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMathChaosEngineView

IMPLEMENT_DYNCREATE(CMathChaosEngineView, CView)

BEGIN_MESSAGE_MAP(CMathChaosEngineView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYDOWN()
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

// CMathChaosEngineView 构造/析构

CMathChaosEngineView::CMathChaosEngineView()noexcept
{
	// TODO: 在此处添加构造代码
	m_timerID = 0;

	//m_engine.setModule(&m_tree);暂时用不上，因为我现在连m_tree都用不上了，后面需要别的Module再改

}

CMathChaosEngineView::~CMathChaosEngineView()
{
}

BOOL CMathChaosEngineView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CMathChaosEngineView 绘图

void CMathChaosEngineView::OnDraw(CDC* pDC)
{


	CMathChaosEngineDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// 使用MFC自带的双缓冲类
	CMemDC memDC(*pDC, this);
	CDC& dc = memDC.GetDC();  // 获取内存DC

	// 获取客户区大小
	CRect rect;
	GetClientRect(&rect);
	m_juliaPanelRect.SetRectEmpty();

	if (m_sceneMode == SCENE_MANDELBROT) {
		m_mandelbrot.render(&dc);
		if (m_juliaLinked) {
			const int panelW = rect.Width() / 3;
			const int panelH = rect.Height() / 3;
			CRect juliaRect(
				rect.right - panelW - 20,
				rect.top + 20,
				rect.right - 20,
				rect.top + 20 + panelH
			);
			m_juliaPanelRect = juliaRect;

			m_julia.render(&dc, juliaRect);

			CPen framePen(PS_SOLID, 1, RGB(245, 245, 245));
			CBrush* oldBrush = (CBrush*)dc.SelectStockObject(NULL_BRUSH);
			CPen* oldPen = dc.SelectObject(&framePen);
			dc.Rectangle(&juliaRect);
			dc.SelectObject(oldPen);
			dc.SelectObject(oldBrush);

			CString juliaInfo;
			CString familyName = _T("Multibrot");
			if (m_julia.getFamily() == JuliaSet::FAMILY_BURNING_SHIP) familyName = _T("BurningShip");
			if (m_julia.getFamily() == JuliaSet::FAMILY_NEWTON) familyName = _T("Newton");
			if (m_julia.getFamily() == JuliaSet::FAMILY_NEWTON) {
				juliaInfo.Format(_T("Newton degree n=%d, roots of z^n=1"), m_julia.getPower());
			}
			else {
				juliaInfo.Format(_T("Julia[%s] n=%d, c = %.4f %+.4fi"), familyName, m_julia.getPower(), m_julia.getConstantReal(), m_julia.getConstantImag());
			}
			dc.SetBkMode(TRANSPARENT);
			dc.SetTextColor(RGB(245, 245, 245));
			dc.TextOut(juliaRect.left + 8, juliaRect.top + 8, juliaInfo);

			if (m_julia.isBoxZoomActive()) {
				CRect jbox = m_julia.getBoxZoomRect();
				if (jbox.left > jbox.right) std::swap(jbox.left, jbox.right);
				if (jbox.top > jbox.bottom) std::swap(jbox.top, jbox.bottom);
				jbox.OffsetRect(juliaRect.left, juliaRect.top);

				CPen jboxPen(PS_DOT, 1, RGB(255, 255, 255));
				CBrush* oldBrush2 = (CBrush*)dc.SelectStockObject(NULL_BRUSH);
				CPen* oldPen2 = dc.SelectObject(&jboxPen);
				dc.Rectangle(&jbox);
				dc.SelectObject(oldPen2);
				dc.SelectObject(oldBrush2);
			}
		}

		if (m_juliaLinked) {
			int sx = 0;
			int sy = 0;
			if (m_mandelbrot.getScreenForComplex(m_julia.getConstantReal(), m_julia.getConstantImag(), sx, sy)) {
				CPen crossPen(PS_SOLID, 1, RGB(255, 255, 255));
				CPen* oldPen = dc.SelectObject(&crossPen);
				dc.MoveTo(sx - 8, sy);
				dc.LineTo(sx + 9, sy);
				dc.MoveTo(sx, sy - 8);
				dc.LineTo(sx, sy + 9);
				dc.SelectObject(oldPen);
			}
		}

		if (m_mandelbrot.isBoxZoomActive()) {
			CRect box = m_mandelbrot.getBoxZoomRect();
			if (box.left > box.right) { std::swap(box.left, box.right); }
			if (box.top > box.bottom) { std::swap(box.top, box.bottom); }

			CPen boxPen(PS_DOT, 1, RGB(255, 255, 255));
			CBrush* oldBrush = (CBrush*)dc.SelectStockObject(NULL_BRUSH);
			CPen* oldPen = dc.SelectObject(&boxPen);
			dc.Rectangle(&box);
			dc.SelectObject(oldPen);
			dc.SelectObject(oldBrush);
		}

		CString stageText = _T("Final");
		switch (m_mandelbrot.getRenderStage()) {
		case Mandelbrot::STAGE_PREVIEW: stageText = _T("Preview"); break;
		case Mandelbrot::STAGE_REFINING: stageText = _T("Refining"); break;
		default: break;
		}

		CString hint;
		CString mainFamilyName = _T("Multibrot");
		if (m_mandelbrot.getFamily() == Mandelbrot::FAMILY_BURNING_SHIP) mainFamilyName = _T("BurningShip");
		if (m_mandelbrot.getFamily() == Mandelbrot::FAMILY_NEWTON) mainFamilyName = _T("Newton");
		hint.Format(
			_T("%s n=%d | 滚轮缩放 | 左拖平移 | Shift+左拖框选 | 单击冻结/恢复c | Backspace回退 | Julia:%s %s | Stage:%s %.0f%% | Zoom:%.2fx"),
			mainFamilyName,
			m_mandelbrot.getPower(),
			m_juliaLinked ? _T("ON") : _T("OFF"),
			m_juliaFrozen ? _T("[Frozen]") : _T("[Live]"),
			stageText,
			m_mandelbrot.getRenderProgress() * 100.0,
			m_mandelbrot.getZoom()
		);
		dc.SetBkMode(TRANSPARENT);
		dc.SetTextColor(RGB(235, 235, 235));
		dc.TextOut(rect.left + 20, rect.top + 20, hint);
		return;
	}

	//// 填充白色背景
	//dc.FillSolidRect(rect, RGB(255, 255, 255));

	int height = rect.Height();

	for (int y = 0; y < height; y++)
	{
		float t = y / (float)height;
		int r, g, b;
		float pulse = (sin(m_time) + 1.0f) * 0.5f;
		if (m_currentTreeMode == LESS_RANDOM) {

			// 渐灰
			//r = (int)(200 + t * 40);
			//g = (int)(200 + t * 40);
			//b = (int)(200 + t * 40);

			r = (int)(200 + t * 40+ pulse * 10);
			g = (int)(200 + t * 40+pulse * 10);
			b = (int)(200 + t * 40+ pulse * 10);
		}
		else {
			// 自然
			//r = (int)(30 + t * 60);
			//g = (int)(50 + t * 120);
			//b = (int)(30 + t * 60);
			r = (int)(30 + t * 60+ pulse * 50);
			g = (int)(50 + t * 120+ pulse * 50);
			b = (int)(30 + t * 60+ pulse * 50);

		}
		CPen pen(PS_SOLID, 1, RGB(r, g, b));
		CPen* oldPen = dc.SelectObject(&pen);

		dc.MoveTo(rect.left, y);
		dc.LineTo(rect.right, y);

		dc.SelectObject(oldPen);
	}

	// 绘制你的内容
	////m_engine.setModule(&m_tree);
	//m_tree.render(&dc);
	for (auto& tree : m_trees) {
		tree.render(&dc);
	}
	// 水印
	CString author = _T("nci1496 | https://github.com/nci1496/MathChaosEngine");

	dc.SetBkMode(TRANSPARENT);

	if (m_currentTreeMode == LESS_RANDOM) {
	dc.SetTextColor(RGB(180, 180, 180));
	}
	else {
		dc.SetTextColor(RGB(160, 240, 160));
	}
	// 放左下角
	dc.TextOut(
		rect.left + 20,
		rect.bottom - 30,
		author
	);
}


// CMathChaosEngineView 打印

BOOL CMathChaosEngineView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CMathChaosEngineView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CMathChaosEngineView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}


// CMathChaosEngineView 诊断

#ifdef _DEBUG
void CMathChaosEngineView::AssertValid() const
{
	CView::AssertValid();
}

void CMathChaosEngineView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMathChaosEngineDoc* CMathChaosEngineView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMathChaosEngineDoc)));
	return (CMathChaosEngineDoc*)m_pDocument;
}
#endif //_DEBUG


// CMathChaosEngineView 消息处理程序

void CMathChaosEngineView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
	// TODO: 在此添加专用代码和/或调用基类
	
	//m_tree.reset();

	//m_engine.setModule(&m_tree);//test only

	////Invalidate(FALSE);

	m_trees.clear();
	m_mandelbrot.reset();
	m_julia.reset();
	m_mandelbrot.setPower(m_fractalPowerN);
	m_julia.setPower(m_fractalPowerN);
	m_mandelbrot.setFamily(m_fractalFamily);
	m_julia.setFamily(static_cast<JuliaSet::FractalFamily>(m_fractalFamily));
	m_juliaFrozen = false;
	m_activeDragTarget = DRAG_NONE;


	m_timerID = SetTimer(1, 50, NULL);

}

void CMathChaosEngineView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	m_time += 0.02f;

	if (nIDEvent == 1) {
		if (m_sceneMode == SCENE_TREE) {
			for (auto& tree : m_trees) {
				tree.update(0.05);
			}
			Invalidate();
		}
		else if (m_sceneMode == SCENE_MANDELBROT) {
			m_mandelbrot.update(0.0);
			m_julia.update(0.0);
			if (m_mandelbrot.consumePixelsChanged() || m_mandelbrot.isRendering() || m_julia.consumePixelsChanged() || m_julia.isRendering()) {
				Invalidate(FALSE);
			}
		}
	}

	CView::OnTimer(nIDEvent);
}

BOOL CMathChaosEngineView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	return TRUE;//把原有的擦除给拦截了
	//return CView::OnEraseBkgnd(pDC);
}

void CMathChaosEngineView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_sceneMode == SCENE_TREE) {
		FractalTree tree;
		tree.mode = m_currentTreeMode;
		tree.reset();
		tree.onMouseDown(point.x, point.y);
		m_trees.push_back(tree);
	}
	else if (m_sceneMode == SCENE_MANDELBROT) {
		SetFocus();
		SetCapture();
		if (m_juliaLinked && m_juliaPanelRect.PtInRect(point)) {
			m_activeDragTarget = DRAG_JULIA;
			const bool boxMode = ((nFlags & MK_SHIFT) != 0);
			m_julia.onMouseDragBegin(point.x - m_juliaPanelRect.left, point.y - m_juliaPanelRect.top, boxMode);
			m_mandelbrotClickCandidate = false;
		}
		else {
			m_activeDragTarget = DRAG_MANDELBROT;
			const bool boxMode = ((nFlags & MK_SHIFT) != 0);
			m_mandelbrot.onMouseDragBegin(point.x, point.y, boxMode);
			m_mandelbrotClickCandidate = !boxMode;
			m_mandelbrotClickStart = point;
		}
	}

	Invalidate(FALSE);

	CView::OnLButtonDown(nFlags, point);
}

void CMathChaosEngineView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_sceneMode == SCENE_MANDELBROT && GetCapture() == this) {
		if (m_activeDragTarget == DRAG_JULIA) {
			m_julia.onMouseDragEnd(point.x - m_juliaPanelRect.left, point.y - m_juliaPanelRect.top);
		}
		else if (m_activeDragTarget == DRAG_MANDELBROT) {
			m_mandelbrot.onMouseDragEnd(point.x, point.y);

			if (m_juliaLinked && m_mandelbrotClickCandidate) {
				const int dx = std::abs(point.x - m_mandelbrotClickStart.x);
				const int dy = std::abs(point.y - m_mandelbrotClickStart.y);
				if (dx <= 3 && dy <= 3) {
					double cx = 0.0;
					double cy = 0.0;
					if (m_mandelbrot.getComplexAtScreen(point.x, point.y, cx, cy)) {
						m_julia.setConstant(cx, cy);
						m_juliaFrozen = !m_juliaFrozen;
					}
				}
			}
		}

		m_activeDragTarget = DRAG_NONE;
		m_mandelbrotClickCandidate = false;
		ReleaseCapture();
		Invalidate(FALSE);
	}

	CView::OnLButtonUp(nFlags, point);
}

void CMathChaosEngineView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_sceneMode == SCENE_MANDELBROT && GetCapture() == this && (nFlags & MK_LBUTTON)) {
		if (m_activeDragTarget == DRAG_JULIA) {
			m_julia.onMouseDragMove(point.x - m_juliaPanelRect.left, point.y - m_juliaPanelRect.top);
		}
		else if (m_activeDragTarget == DRAG_MANDELBROT) {
			m_mandelbrot.onMouseDragMove(point.x, point.y);

			const int dx = std::abs(point.x - m_mandelbrotClickStart.x);
			const int dy = std::abs(point.y - m_mandelbrotClickStart.y);
			if (dx > 3 || dy > 3) {
				m_mandelbrotClickCandidate = false;
			}

			if (m_juliaLinked && !m_juliaFrozen) {
				double cx = 0.0;
				double cy = 0.0;
				if (m_mandelbrot.getComplexAtScreen(point.x, point.y, cx, cy)) {
					m_julia.setConstant(cx, cy);
				}
			}
		}
		Invalidate(FALSE);
	}
	else if (m_sceneMode == SCENE_MANDELBROT && m_juliaLinked && !m_juliaFrozen && !m_juliaPanelRect.PtInRect(point)) {
		double cx = 0.0;
		double cy = 0.0;
		if (m_mandelbrot.getComplexAtScreen(point.x, point.y, cx, cy)) {
			m_julia.setConstant(cx, cy);
			Invalidate(FALSE);
		}
	}

	CView::OnMouseMove(nFlags, point);
}

BOOL CMathChaosEngineView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (m_sceneMode == SCENE_MANDELBROT) {
		CPoint clientPt = pt;
		ScreenToClient(&clientPt);
		if (m_juliaLinked && m_juliaPanelRect.PtInRect(clientPt)) {
			m_julia.onMouseWheel(clientPt.x - m_juliaPanelRect.left, clientPt.y - m_juliaPanelRect.top, zDelta);
		}
		else {
			m_mandelbrot.onMouseWheel(clientPt.x, clientPt.y, zDelta);
		}
		Invalidate(FALSE);
		return TRUE;
	}

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CMathChaosEngineView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (m_sceneMode == SCENE_MANDELBROT && nChar == VK_BACK) {
		m_mandelbrot.goBack();
		Invalidate(FALSE);
		return;
	}

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CMathChaosEngineView::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CMenu menu;
	CMenu menuPalette;
	CMenu menuQuality;
	CMenu menuPower;
	CMenu menuFamily;
	menu.CreatePopupMenu();

	menu.AppendMenu(MF_STRING, 10, _T("树：经典"));
	menu.AppendMenu(MF_STRING, 11, _T("树：自然（加强随机）"));
	menu.AppendMenu(MF_STRING, 12, _T("Mandelbrot 集"));
	menu.AppendMenu(MF_SEPARATOR);
	if (m_sceneMode == SCENE_TREE) {
		menu.AppendMenu(MF_STRING, 20, _T("清空画布"));
	}
	else if (m_sceneMode == SCENE_MANDELBROT) {
		menuPalette.CreatePopupMenu();
		menuPalette.AppendMenu(MF_STRING, 31, _T("科学清晰"));
		menuPalette.AppendMenu(MF_STRING, 32, _T("霓虹艺术"));
		menuPalette.AppendMenu(MF_STRING, 33, _T("黑白版画"));

		menuQuality.CreatePopupMenu();
		menuQuality.AppendMenu(MF_STRING, 41, _T("速度优先"));
		menuQuality.AppendMenu(MF_STRING, 42, _T("平衡"));
		menuQuality.AppendMenu(MF_STRING, 43, _T("细节优先"));

		menuPower.CreatePopupMenu();
		menuPower.AppendMenu(MF_STRING, 51, _T("n = 2（经典）"));
		menuPower.AppendMenu(MF_STRING, 52, _T("n = 3"));
		menuPower.AppendMenu(MF_STRING, 53, _T("n = 4"));
		menuPower.AppendMenu(MF_STRING, 54, _T("n = 5"));

		menuFamily.CreatePopupMenu();
		menuFamily.AppendMenu(MF_STRING, 61, _T("Multibrot"));
		menuFamily.AppendMenu(MF_STRING, 62, _T("Burning Ship"));
		menuFamily.AppendMenu(MF_STRING, 63, _T("Newton Fractal"));

		menu.AppendMenu(MF_STRING, 21, _T("重置视图"));
		menu.AppendMenu(MF_STRING, 22, _T("缩小一级"));
		menu.AppendMenu(MF_STRING, 23, _T("回退一步"));
		menu.AppendMenu(MF_STRING, 24, _T("返回 Home"));
		menu.AppendMenu(MF_STRING, 25, _T("Julia 联动开关"));
		menu.AppendMenu(MF_STRING, 26, _T("Julia 冻结/恢复 c"));
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_POPUP, (UINT_PTR)menuFamily.GetSafeHmenu(), _T("分形族"));
		menu.AppendMenu(MF_POPUP, (UINT_PTR)menuPower.GetSafeHmenu(), _T("幂次 n"));
		menu.AppendMenu(MF_POPUP, (UINT_PTR)menuPalette.GetSafeHmenu(), _T("风格"));
		menu.AppendMenu(MF_POPUP, (UINT_PTR)menuQuality.GetSafeHmenu(), _T("画质"));
		if (m_juliaLinked) menu.CheckMenuItem(25, MF_CHECKED);
		if (m_juliaFrozen) menu.CheckMenuItem(26, MF_CHECKED);

		Mandelbrot::PaletteId palette = m_mandelbrot.getPalette();
		if (palette == Mandelbrot::PALETTE_SCIENTIFIC) menuPalette.CheckMenuItem(31, MF_CHECKED);
		if (palette == Mandelbrot::PALETTE_NEON) menuPalette.CheckMenuItem(32, MF_CHECKED);
		if (palette == Mandelbrot::PALETTE_MONO) menuPalette.CheckMenuItem(33, MF_CHECKED);

		Mandelbrot::QualityPreset quality = m_mandelbrot.getQuality();
		if (quality == Mandelbrot::QUALITY_FAST) menuQuality.CheckMenuItem(41, MF_CHECKED);
		if (quality == Mandelbrot::QUALITY_BALANCED) menuQuality.CheckMenuItem(42, MF_CHECKED);
		if (quality == Mandelbrot::QUALITY_DETAIL) menuQuality.CheckMenuItem(43, MF_CHECKED);

		int power = m_mandelbrot.getPower();
		if (power == 2) menuPower.CheckMenuItem(51, MF_CHECKED);
		if (power == 3) menuPower.CheckMenuItem(52, MF_CHECKED);
		if (power == 4) menuPower.CheckMenuItem(53, MF_CHECKED);
		if (power == 5) menuPower.CheckMenuItem(54, MF_CHECKED);

		if (m_mandelbrot.getFamily() == Mandelbrot::FAMILY_MULTIBROT) menuFamily.CheckMenuItem(61, MF_CHECKED);
		if (m_mandelbrot.getFamily() == Mandelbrot::FAMILY_BURNING_SHIP) menuFamily.CheckMenuItem(62, MF_CHECKED);
		if (m_mandelbrot.getFamily() == Mandelbrot::FAMILY_NEWTON) menuFamily.CheckMenuItem(63, MF_CHECKED);
	}

	// 转换为屏幕坐标（必须！）
	ClientToScreen(&point);

	int cmd = menu.TrackPopupMenu(
		TPM_RETURNCMD | TPM_LEFTALIGN,
		point.x, point.y,
		this
	);

	switch (cmd)
	{
	case 10: // 树：经典
		m_sceneMode = SCENE_TREE;
		m_currentTreeMode = LESS_RANDOM;
		m_trees.clear();
		break;

	case 11: // 树：自然
		m_sceneMode = SCENE_TREE;
		m_currentTreeMode = MORE_RANDOM;
		m_trees.clear();
		break;

	case 12: // Mandelbrot
		m_sceneMode = SCENE_MANDELBROT;
		break;

	case 20: // 清空
		m_trees.clear();
		break;

	case 21: // Mandelbrot reset
		m_mandelbrot.reset();
		break;

	case 22: // Mandelbrot zoom out
		m_mandelbrot.zoomOut();
		break;

	case 23: // Mandelbrot back
		m_mandelbrot.goBack();
		break;

	case 24: // Mandelbrot home
		m_mandelbrot.goHome();
		break;

	case 25:
		m_juliaLinked = !m_juliaLinked;
		if (!m_juliaLinked) m_juliaFrozen = false;
		break;

	case 26:
		m_juliaFrozen = !m_juliaFrozen;
		break;

	case 31:
		m_mandelbrot.setPalette(Mandelbrot::PALETTE_SCIENTIFIC);
		m_julia.setPalette(JuliaSet::PALETTE_SCIENTIFIC);
		break;
	case 32:
		m_mandelbrot.setPalette(Mandelbrot::PALETTE_NEON);
		m_julia.setPalette(JuliaSet::PALETTE_NEON);
		break;
	case 33:
		m_mandelbrot.setPalette(Mandelbrot::PALETTE_MONO);
		m_julia.setPalette(JuliaSet::PALETTE_MONO);
		break;

	case 41:
		m_mandelbrot.setQuality(Mandelbrot::QUALITY_FAST);
		m_julia.setQuality(JuliaSet::QUALITY_FAST);
		break;
	case 42:
		m_mandelbrot.setQuality(Mandelbrot::QUALITY_BALANCED);
		m_julia.setQuality(JuliaSet::QUALITY_BALANCED);
		break;
	case 43:
		m_mandelbrot.setQuality(Mandelbrot::QUALITY_DETAIL);
		m_julia.setQuality(JuliaSet::QUALITY_DETAIL);
		break;

	case 51:
		m_fractalPowerN = 2;
		m_mandelbrot.setPower(m_fractalPowerN);
		m_julia.setPower(m_fractalPowerN);
		break;
	case 52:
		m_fractalPowerN = 3;
		m_mandelbrot.setPower(m_fractalPowerN);
		m_julia.setPower(m_fractalPowerN);
		break;
	case 53:
		m_fractalPowerN = 4;
		m_mandelbrot.setPower(m_fractalPowerN);
		m_julia.setPower(m_fractalPowerN);
		break;
	case 54:
		m_fractalPowerN = 5;
		m_mandelbrot.setPower(m_fractalPowerN);
		m_julia.setPower(m_fractalPowerN);
		break;

	case 61:
		m_fractalFamily = Mandelbrot::FAMILY_MULTIBROT;
		m_mandelbrot.setFamily(m_fractalFamily);
		m_julia.setFamily(static_cast<JuliaSet::FractalFamily>(m_fractalFamily));
		break;
	case 62:
		m_fractalFamily = Mandelbrot::FAMILY_BURNING_SHIP;
		m_mandelbrot.setFamily(m_fractalFamily);
		m_julia.setFamily(static_cast<JuliaSet::FractalFamily>(m_fractalFamily));
		break;
	case 63:
		m_fractalFamily = Mandelbrot::FAMILY_NEWTON;
		m_mandelbrot.setFamily(m_fractalFamily);
		m_julia.setFamily(static_cast<JuliaSet::FractalFamily>(m_fractalFamily));
		break;
	}

	Invalidate(FALSE);

	CView::OnRButtonDown(nFlags, point);
}
