
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
END_MESSAGE_MAP()

// CMathChaosEngineView 构造/析构

CMathChaosEngineView::CMathChaosEngineView() noexcept
{
	// TODO: 在此处添加构造代码

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

void CMathChaosEngineView::OnDraw(CDC* /*pDC*/)
{
	CMathChaosEngineDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码
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
