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

    mode = LESS_RANDOM;

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
            branch.progress = std::max<double>(0.0, std::min<double>(1.0, branchProgress));

        }
    }
}


void FractalTree::render(CDC* pDC)
{

    CRect rect;
    pDC->GetClipBox(&rect);
    viewSize = Vec2(rect.Width(), rect.Height());

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
    if (mode == LESS_RANDOM) {
        generateBranchesLessRandom(rootPos, baseLength, -M_PI / 2, maxDepth);
    }
    else {
        generateBranchesMoreRandom(rootPos, baseLength, -M_PI / 2, maxDepth);
    }   
}



void FractalTree::generateBranchesLessRandom(Vec2 pos, double length, double angle, int depth)
{

    if (depth <= 0) return;

    // 计算终点
    Vec2 end = pos + Vec2(length * cos(angle), length * sin(angle));

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
    generateBranchesLessRandom(end, newLength, angle + branchAngle + angleOffset, depth - 1);
    generateBranchesLessRandom(end, newLength, angle - branchAngle + angleOffset, depth - 1);

}


void FractalTree::generateBranchesMoreRandom(Vec2 pos, double length, double angle, int depth)
{
    if (depth <= 0 || length < 3 || branches.size() > 2000)
        return;

    Vec2 end = pos + Vec2(length * cos(angle), length * sin(angle));
    addBranch(pos, end, depth, length, angle);

    double newLength = length * g_Random.range(0.7, 0.78);

    int level = maxDepth - depth;

    int childCount;

    // 前期
    if (level < 2)
        childCount = 2;

    // 中期：稍微丰富
    else if (level < 5)
        childCount = g_Random.range(2, 3);

    // 后期：减少密度
    else
    {
        if (g_Random.range(0.0, 1.0) <= 0.05) { return; }//随机筛
        childCount = g_Random.range(1, 2);
    }
    // 扇形展开

    double levelRatio = (maxDepth - depth) / (double)maxDepth;

    double spread = branchAngle * (1.2 + 5*levelRatio);

    for (int i = 0; i < childCount; i++)
    {
        double t = (childCount == 1) ? 0.5 : (double)i / (childCount - 1);

        double baseOffset = -spread / 2 + t * spread;

        // 小扰动
        double jitter = g_Random.range(-0.15, 0.15) * branchAngle;

        double newAngle = angle + baseOffset + jitter;

        generateBranchesMoreRandom(end, newLength, newAngle, depth - 1);
    }
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
