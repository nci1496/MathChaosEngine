
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

	if (m_sceneMode == SCENE_MANDELBROT) {
		m_mandelbrot.render(&dc);

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
		hint.Format(
			_T("Mandelbrot | 滚轮缩放 | 左拖平移 | Shift+左拖框选 | Backspace回退 | Stage:%s %.0f%% | Zoom:%.2fx"),
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
			if (m_mandelbrot.consumePixelsChanged() || m_mandelbrot.isRendering()) {
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
		const bool boxMode = ((nFlags & MK_SHIFT) != 0);
		m_mandelbrot.onMouseDragBegin(point.x, point.y, boxMode);
	}

	Invalidate(FALSE);

	CView::OnLButtonDown(nFlags, point);
}

void CMathChaosEngineView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_sceneMode == SCENE_MANDELBROT && GetCapture() == this) {
		m_mandelbrot.onMouseDragEnd(point.x, point.y);
		ReleaseCapture();
		Invalidate(FALSE);
	}

	CView::OnLButtonUp(nFlags, point);
}

void CMathChaosEngineView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_sceneMode == SCENE_MANDELBROT && GetCapture() == this && (nFlags & MK_LBUTTON)) {
		m_mandelbrot.onMouseDragMove(point.x, point.y);
		Invalidate(FALSE);
	}

	CView::OnMouseMove(nFlags, point);
}

BOOL CMathChaosEngineView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (m_sceneMode == SCENE_MANDELBROT) {
		CPoint clientPt = pt;
		ScreenToClient(&clientPt);
		m_mandelbrot.onMouseWheel(clientPt.x, clientPt.y, zDelta);
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

		menu.AppendMenu(MF_STRING, 21, _T("重置视图"));
		menu.AppendMenu(MF_STRING, 22, _T("缩小一级"));
		menu.AppendMenu(MF_STRING, 23, _T("回退一步"));
		menu.AppendMenu(MF_STRING, 24, _T("返回 Home"));
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_POPUP, (UINT_PTR)menuPalette.GetSafeHmenu(), _T("风格"));
		menu.AppendMenu(MF_POPUP, (UINT_PTR)menuQuality.GetSafeHmenu(), _T("画质"));

		Mandelbrot::PaletteId palette = m_mandelbrot.getPalette();
		if (palette == Mandelbrot::PALETTE_SCIENTIFIC) menuPalette.CheckMenuItem(31, MF_CHECKED);
		if (palette == Mandelbrot::PALETTE_NEON) menuPalette.CheckMenuItem(32, MF_CHECKED);
		if (palette == Mandelbrot::PALETTE_MONO) menuPalette.CheckMenuItem(33, MF_CHECKED);

		Mandelbrot::QualityPreset quality = m_mandelbrot.getQuality();
		if (quality == Mandelbrot::QUALITY_FAST) menuQuality.CheckMenuItem(41, MF_CHECKED);
		if (quality == Mandelbrot::QUALITY_BALANCED) menuQuality.CheckMenuItem(42, MF_CHECKED);
		if (quality == Mandelbrot::QUALITY_DETAIL) menuQuality.CheckMenuItem(43, MF_CHECKED);
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

	case 31:
		m_mandelbrot.setPalette(Mandelbrot::PALETTE_SCIENTIFIC);
		break;
	case 32:
		m_mandelbrot.setPalette(Mandelbrot::PALETTE_NEON);
		break;
	case 33:
		m_mandelbrot.setPalette(Mandelbrot::PALETTE_MONO);
		break;

	case 41:
		m_mandelbrot.setQuality(Mandelbrot::QUALITY_FAST);
		break;
	case 42:
		m_mandelbrot.setQuality(Mandelbrot::QUALITY_BALANCED);
		break;
	case 43:
		m_mandelbrot.setQuality(Mandelbrot::QUALITY_DETAIL);
		break;
	}

	Invalidate(FALSE);

	CView::OnRButtonDown(nFlags, point);
}
