
// MathChaosEngineView.h: CMathChaosEngineView 类的接口
//

#pragma once

#include "core/Engine.h"
#include "modules/FractalTree.h"


class CMathChaosEngineView : public CView
{
protected: // 仅从序列化创建
	CMathChaosEngineView() noexcept;
	DECLARE_DYNCREATE(CMathChaosEngineView)

	//成员
protected:
    Engine m_engine;


	std::vector<FractalTree>m_trees;
	TreeMode m_currentTreeMode = LESS_RANDOM;

	int m_timerID;

	float m_time = 0.0f;//用于动态背景 //test only

// 特性
public:
	CMathChaosEngineDoc* GetDocument() const;

// 操作
public:

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 实现
public:
	virtual ~CMathChaosEngineView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
};

#ifndef _DEBUG  // MathChaosEngineView.cpp 中的调试版本
inline CMathChaosEngineDoc* CMathChaosEngineView::GetDocument() const
   { return reinterpret_cast<CMathChaosEngineDoc*>(m_pDocument); }
#endif

