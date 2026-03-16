
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
END_MESSAGE_MAP()

// CMathChaosEngineView 构造/析构

CMathChaosEngineView::CMathChaosEngineView()noexcept
{
	// TODO: 在此处添加构造代码
	m_timerID = 0;

	m_engine.setModule(&m_tree);

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

	// 填充白色背景
	dc.FillSolidRect(rect, RGB(255, 255, 255));

	// 绘制你的内容
	//m_engine.setModule(&m_tree);
	m_tree.render(&dc);
	

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

	// 获取窗口大小
	//CRect rect;
	//GetClientRect(&rect);

	//CString msg;
	//msg.Format(_T("窗口大小: %d x %d"), rect.Width(), rect.Height());
	//AfxMessageBox(msg);  // 弹出消息框显示尺寸

	// TODO: 在此添加专用代码和/或调用基类
	
	m_tree.reset();

	m_engine.setModule(&m_tree);//test only

	Invalidate(FALSE);
	m_timerID = SetTimer(1, 50, NULL);

}

void CMathChaosEngineView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (nIDEvent == 1) {
		m_tree.update(0.05);
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

	m_tree.onMouseDown(point.x, point.y);

	Invalidate(FALSE);

	CView::OnLButtonDown(nFlags, point);
}
