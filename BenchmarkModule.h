#pragma once
#include <string>
#include <vector>

struct BenchResult {
    std::string label;
    std::string result;
    double      timeMs;
    std::string algorithm;
    BenchResult() : timeMs(0.0) {}
};

class BenchmarkModule {
public:
    static std::vector<BenchResult> runAll();

private:
    static BenchResult benchCombination(int n, int r);
    static BenchResult benchSetUnion(int size);
    static BenchResult benchSetIntersection(int size);
    static BenchResult benchBitmaskSubsets(int n);
};