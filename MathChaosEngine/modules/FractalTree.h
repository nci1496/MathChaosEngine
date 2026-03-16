#pragma once

#include"../core/Module.h"
#include"../math/ColorGradient.h"
#include"../math/Random.h"
#include"../math/Vec2.h"

#include<vector>


class FractalTree : public Module
{
private:
    struct Branch {
        Vec2 start;
        Vec2 end;
        int depth;
        double length;
        double angle;
        double progress;  // 0~1 生长进度

        Vec2 currentEnd() const {
            return start.lerp(end, progress);
        }
    };

    std::vector<Branch> branches;  // 存储所有树枝
    double growthProgress;         // 整体生长进度
    double maxProgress;            // 最大生长进度

    int maxDepth;
    double baseLength;
    double branchAngle;

    Vec2 rootPos;//根的位置

    ColorGradient colorScheme;//预定义颜色

    void generateBranches(Vec2 pos, double length, double angle, int depth);
    void addBranch(const Vec2& start, const Vec2& end, int depth, double len, double ang);

public:

    Vec2 viewSize;

    FractalTree();

    void reset() override;
    void update(double dt)override;
    void render(CDC* pDC) override;
    
    void onMouseDown(int x,int y)override;

    void setColorScheme(const ColorGradient& scheme);
    void setRandomColorScheme();
    void randomizeParameters();  // 随机化参数

    void setRoot(Vec2 pos);

//private:
//
//    void drawBranch(CDC* pDC, double x, double y, double length, double angle, int depth);
//

};
