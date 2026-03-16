// math/ColorGradient.h
#pragma once
#include <vector>

#include<algorithm>

struct ColorPoint {
    double position;  // 0.0 ~ 1.0
    int r, g, b;
};

class ColorGradient {
private:
    std::vector<ColorPoint> points;

public:
    ColorGradient() {
        // 默认：绿色到蓝色渐变
        addPoint(0.0, 34, 139, 34);   // 森林绿
        addPoint(0.5, 70, 130, 180);  // 钢蓝
        addPoint(1.0, 25, 25, 112);   // 午夜蓝
    }

    void addPoint(double pos, int r, int g, int b) {
        points.push_back({ pos, r, g, b });
        // 按位置排序
        std::sort(points.begin(), points.end(),
            [](const ColorPoint& a, const ColorPoint& b) {
                return a.position < b.position;
            });
    }

    COLORREF getColor(double t) const {
        if (points.empty()) return RGB(0, 0, 0);
        if (t <= points.front().position)
            return RGB(points.front().r, points.front().g, points.front().b);
        if (t >= points.back().position)
            return RGB(points.back().r, points.back().g, points.back().b);

        // 找到 t 所在的区间
        for (size_t i = 0; i < points.size() - 1; ++i) {
            if (t >= points[i].position && t <= points[i + 1].position) {
                double localT = (t - points[i].position) /
                    (points[i + 1].position - points[i].position);

                int r = points[i].r + (points[i + 1].r - points[i].r) * localT;
                int g = points[i].g + (points[i + 1].g - points[i].g) * localT;
                int b = points[i].b + (points[i + 1].b - points[i].b) * localT;

                return RGB(r, g, b);
            }
        }
        return RGB(0, 0, 0);
    }

    // 预定义优雅的渐变色系
    static ColorGradient sunset() {
        ColorGradient g;
        g.points.clear();
        g.addPoint(0.0, 255, 94, 77);   // 珊瑚红
        g.addPoint(0.5, 255, 159, 67);  // 橙黄
        g.addPoint(1.0, 255, 221, 89);  // 淡黄
        return g;
    }

    static ColorGradient ocean() {
        ColorGradient g;
        g.points.clear();
        g.addPoint(0.0, 0, 168, 150);   // 碧绿
        g.addPoint(0.5, 0, 153, 204);   // 天蓝
        g.addPoint(1.0, 46, 134, 222);  // 深蓝
        return g;
    }

    static ColorGradient forest() {
        ColorGradient g;
        g.points.clear();
        g.addPoint(0.0, 85, 107, 47);   // 橄榄绿
        g.addPoint(0.5, 124, 179, 66);  // 草绿
        g.addPoint(1.0, 193, 225, 107); // 嫩绿
        return g;
    }

    static ColorGradient lavender() {
        ColorGradient g;
        g.points.clear();
        g.addPoint(0.0, 142, 68, 173);  // 紫罗兰
        g.addPoint(0.5, 187, 134, 252); // 淡紫
        g.addPoint(1.0, 255, 182, 255); // 粉紫
        return g;
    }
};