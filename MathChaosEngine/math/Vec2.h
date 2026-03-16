#pragma once
#include <cmath>

struct Vec2 {
    double x, y;

    Vec2() : x(0), y(0) {}
    Vec2(double x, double y) : x(x), y(y) {}

    // 向量运算
    Vec2 operator+(const Vec2& v) const { return Vec2(x + v.x, y + v.y); }
    Vec2 operator-(const Vec2& v) const { return Vec2(x - v.x, y - v.y); }
    Vec2 operator*(double s) const { return Vec2(x * s, y * s); }
    Vec2 operator/(double s) const { return Vec2(x / s, y / s); }

    // 点积
    double dot(const Vec2& v) const { return x * v.x + y * v.y; }

    // 长度
    double length() const { return std::sqrt(x * x + y * y); }
    double lengthSq() const { return x * x + y * y; }

    // 归一化
    Vec2 normalized() const {
        double len = length();
        if (len > 0) return *this / len;
        return *this;
    }

    // 旋转
    Vec2 rotated(double angle) const {
        double c = cos(angle);
        double s = sin(angle);
        return Vec2(x * c - y * s, x * s + y * c);
    }

    // 角度
    double angle() const { return std::atan2(y, x); }

    // 距离
    double distanceTo(const Vec2& v) const { return (*this - v).length(); }

    // 线性插值
    Vec2 lerp(const Vec2& v, double t) const {
        return *this * (1 - t) + v * t;
    }
};