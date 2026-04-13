
// MathChaosEngineView.h: CMathChaosEngineView 类的接口
//

#pragma once

#include "core/Engine.h"
#include "modules/FractalTree.h"
#include "modules/Mandelbrot.h"
#include "modules/Julia.h"


class CMathChaosEngineView : public CView
{
protected: // 仅从序列化创建
	CMathChaosEngineView() noexcept;
	DECLARE_DYNCREATE(CMathChaosEngineView)

	//成员
protected:
    Engine m_engine;

    enum SceneMode {
        SCENE_TREE = 0,
        SCENE_MANDELBROT = 1
    };

    SceneMode m_sceneMode = SCENE_TREE;
    Mandelbrot m_mandelbrot;
    JuliaSet m_julia;
    int m_fractalPowerN = 2;
    Mandelbrot::FractalFamily m_fractalFamily = Mandelbrot::FAMILY_MULTIBROT;
    bool m_juliaLinked = true;
    bool m_juliaFrozen = false;
    CRect m_juliaPanelRect;
    bool m_mandelbrotClickCandidate = false;
    CPoint m_mandelbrotClickStart;

    enum ActiveDragTarget {
        DRAG_NONE = 0,
        DRAG_MANDELBROT = 1,
        DRAG_JULIA = 2
    };
    ActiveDragTarget m_activeDragTarget = DRAG_NONE;

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
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
};

#ifndef _DEBUG  // MathChaosEngineView.cpp 中的调试版本
inline CMathChaosEngineDoc* CMathChaosEngineView::GetDocument() const
   { return reinterpret_cast<CMathChaosEngineDoc*>(m_pDocument); }
#endif

