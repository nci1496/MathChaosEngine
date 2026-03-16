#include "pch.h" 
#include "FractalTree.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include<algorithm>

Random g_Random;



FractalTree::FractalTree():growthProgress(0),maxProgress(1.0),maxDepth(g_Random.range(7,10)),baseLength(120),branchAngle(25.0*M_PI/180.0)

{
    viewSize = Vec2(1200, 800);
    colorScheme = ColorGradient::forest();

}

void FractalTree::reset() {
    growthProgress = 0;
    branches.clear();

}

void FractalTree::update(double dt)
{
    if (growthProgress < maxProgress) {
        growthProgress += dt * 0.5;

        for (auto& branch : branches) {
            double depthDelay = (maxDepth - branch.depth) * 0.1;
            double branchProgress = (growthProgress - depthDelay) * 1.5;
            /*branch.progress = std::max(0.0, std::min(1.0, branchProgress));*/
            branch.progress = std::max<double>(0.0, std::min<double>(1.0, branchProgress));

        }
    }
}


void FractalTree::render(CDC* pDC)
{

    TRACE(_T("branches size = %d\n"), branches.size());

    CRect rect;
    pDC->GetClipBox(&rect);
    viewSize = Vec2(rect.Width(), rect.Height());

    growthProgress += 0.01;

    if (growthProgress > maxProgress)
    {
        growthProgress = maxProgress;
    }
    CPen pen;
    CPen* oldPen = nullptr;

    for (auto& branch : branches) {
        double delay = (maxDepth - branch.depth) / (double)maxDepth;
        double progress = growthProgress - delay;



        if (progress <= 0) { continue; }

        if (progress >1 ) { progress = 1; }

        branch.progress = progress;

        // 颜色计算
        double depthFactor = 1.0 - (branch.depth / (double)maxDepth);
        double colorT = depthFactor * 0.7 + branch.progress * 0.3;
        COLORREF color = colorScheme.getColor(colorT);
        // 当前端点
        Vec2 currentEnd = branch.currentEnd();

        // 画笔粗细
        int penWidth = std::max<int>(1, (int)(branch.depth * 1.5 * branch.progress));
        
        pen.DeleteObject();
        pen.CreatePen(PS_SOLID, penWidth, color);
        oldPen = pDC->SelectObject(&pen);

        pDC->MoveTo((int)branch.start.x, (int)branch.start.y);
        pDC->LineTo((int)currentEnd.x, (int)currentEnd.y);

        pDC->SelectObject(oldPen);
    }


}

void FractalTree::generateBranches(Vec2 pos, double length, double angle, int depth)
{

    //TRACE(_T("generateBranches: depth=%d, pos=(%.1f,%.1f)\n"), depth, pos.x, pos.y);

    if (depth <= 0) return;

    // 计算终点
    Vec2 end = pos + Vec2(length * cos(angle), length * sin(angle));


    //TRACE(_T("  终点: (%.1f,%.1f)\n"), end.x, end.y);

    if (depth <= 0 || length < 3)
    {
        return;
    }
    if (branches.size() > 5000)
    {
        return;
    }

    // 存储这条树枝
    addBranch(pos, end, depth, length, angle);

    // 随机变化：长度和角度
    double newLength = length * g_Random.range(0.65, 0.75);
    double angleOffset = g_Random.range(-5.0, 5.0) * M_PI / 180.0;

    // 递归生成左右分支
    generateBranches(end, newLength, angle + branchAngle + angleOffset, depth - 1);
    generateBranches(end, newLength, angle - branchAngle + angleOffset, depth - 1);

}

void FractalTree::addBranch(const Vec2& start, const Vec2& end, int depth, double len, double ang)
{
    Branch br;
    br.start = start;
    br.end = end;
    br.depth = depth;
    br.length = len;
    br.angle = ang;
    br.progress = 0;
    branches.push_back(br);
    TRACE(_T("branch added depth=%d total=%d\n"), depth, branches.size());

}


void FractalTree::onMouseDown(int x, int y)
{
    reset();

    rootPos = Vec2(x, y);

    setRandomColorScheme();
    randomizeParameters();  // 随机化所有参数
    generateBranches(rootPos, baseLength, -M_PI / 2, maxDepth);
    TRACE(_T("Total branches generated = %d\n"), branches.size());

}

void FractalTree::setColorScheme(const ColorGradient& scheme)
{
    colorScheme = scheme;
}

void FractalTree::setRandomColorScheme()
{
    int choice = g_Random.range(0, 3);
    switch (choice) {
    case 0: colorScheme = ColorGradient::sunset(); break;
    case 1: colorScheme = ColorGradient::ocean(); break;
    case 2: colorScheme = ColorGradient::forest(); break;
    case 3: colorScheme = ColorGradient::lavender(); break;
    }

}

void FractalTree::randomizeParameters()
{
    // 使用统一的Random类
    branchAngle = g_Random.range(20.0, 35.0) * M_PI / 180.0;
    baseLength = g_Random.range(80.0, 150.0);
    maxDepth = g_Random.range(8, 14);
}

void FractalTree::setRoot(Vec2 pos)
{
    rootPos = pos;
    branches.clear();
    generateBranches(rootPos, baseLength, -M_PI / 2, maxDepth);

}




//void FractalTree::drawBranch(CDC* pDC, double x, double y, double length, double ang, int depth)
//{
//    if (depth <= 0)
//    {
//        return;
//    }
//
//    double x2 = x + length * cos(ang);
//    double y2 = y + length * sin(ang);
//
//    // 根据深度设置颜色和粗细
//    CPen pen(PS_SOLID, depth, RGB(0, 128, 0));  // 绿色，深度越大越粗
//    CPen* pOldPen = pDC->SelectObject(&pen);
//
//    pDC->MoveTo((int)x, (int)y);
//    pDC->LineTo((int)x2, (int)y2);
//
//    pDC->SelectObject(pOldPen);
//    
//    drawBranch(pDC, x2, y2, length * 0.7, ang + angle, depth - 1);
//    drawBranch(pDC, x2, y2, length * 0.7, ang - angle, depth - 1);
//}