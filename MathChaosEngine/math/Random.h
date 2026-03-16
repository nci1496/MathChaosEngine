#pragma once
#include <random>
#include <chrono>

class Random {
private:
    std::mt19937 rng;

public:
    Random() {
        // 使用时间种子
        rng.seed(std::chrono::steady_clock::now().time_since_epoch().count());
    }

    // 整数范围 [min, max]
    int range(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(rng);
    }

    // 浮点数范围 [min, max]
    double range(double min, double max) {
        std::uniform_real_distribution<double> dist(min, max);
        return dist(rng);
    }

    // 0~1 浮点数
    double unit() {
        return range(0.0, 1.0);
    }

    // 正态分布
    double gaussian(double mean = 0.0, double stddev = 1.0) {
        std::normal_distribution<double> dist(mean, stddev);
        return dist(rng);
    }

    // 获取随机引擎（用于需要直接使用引擎的场景）
    std::mt19937& getEngine() { return rng; }
};

// 全局随机实例（在某个cpp中定义）
extern Random g_Random;