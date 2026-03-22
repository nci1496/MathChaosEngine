
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
	ON_WM_RBUTTONDOWN()
//	ON_WM_KEYDOWN()
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


	m_timerID = SetTimer(1, 50, NULL);

}

void CMathChaosEngineView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	m_time += 0.02f;

	if (nIDEvent == 1) {
		for (auto& tree : m_trees) {
			tree.update(0.05);
		}
		Invalidate();  // 触发重绘
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
		// 获取窗口大小

	//m_tree.onMouseDown(point.x, point.y);
	FractalTree tree;
	tree.mode = m_currentTreeMode;
	tree.reset();
	tree.onMouseDown(point.x, point.y);

	m_trees.push_back(tree);

	Invalidate(FALSE);

	CView::OnLButtonDown(nFlags, point);
}

void CMathChaosEngineView::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CMenu menu;
	menu.CreatePopupMenu();

	menu.AppendMenu(MF_STRING, 1, _T("经典"));
	menu.AppendMenu(MF_STRING, 2, _T("自然（加强随机）"));
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, 3, _T("清空"));

	// 转换为屏幕坐标（必须！）
	ClientToScreen(&point);

	int cmd = menu.TrackPopupMenu(
		TPM_RETURNCMD | TPM_LEFTALIGN,
		point.x, point.y,
		this
	);

	switch (cmd)
	{
	case 1: // 经典
		m_currentTreeMode = LESS_RANDOM;
		m_trees.clear(); // 切换时清空更干净
		break;

	case 2: // 自然
		m_currentTreeMode = MORE_RANDOM;
		m_trees.clear();
		break;

	case 3: // 清空
		m_trees.clear();
		break;
	}

	Invalidate(FALSE);

	CView::OnRButtonDown(nFlags, point);
}
