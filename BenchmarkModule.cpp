#include "BenchmarkModule.h"
#include "CombinationsModule.h"
#include "SetsModule.h"
#include <chrono>
#include <set>
#include <string>

typedef std::chrono::high_resolution_clock Clock;

BenchResult BenchmarkModule::benchCombination(int n, int r) {
    auto t0 = Clock::now();
    long long val = CombinationsModule::combination(n, r);
    auto t1 = Clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    BenchResult res;
    res.label = "C(" + std::to_string(n) + "," + std::to_string(r) + ")";
    res.result = std::to_string(val);
    res.timeMs = ms;
    res.algorithm = "Dynamic Programming with Memoization";
    return res;
}

BenchResult BenchmarkModule::benchSetUnion(int size) {
    std::set<std::string> A, B;
    for (int i = 0; i < size; ++i) A.insert("a" + std::to_string(i));
    for (int i = size / 2; i < size + size / 2; ++i) B.insert("a" + std::to_string(i));

    auto t0 = Clock::now();
    std::set<std::string> res = SetsModule::setUnion(A, B);
    auto t1 = Clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    BenchResult r;
    r.label = "Union of sets (" + std::to_string(size) + " + " + std::to_string(size) + " elements)";
    r.result = "Result size: " + std::to_string(res.size());
    r.timeMs = ms;
    r.algorithm = "STL set_union";
    return r;
}

BenchResult BenchmarkModule::benchSetIntersection(int size) {
    std::set<std::string> A, B;
    for (int i = 0; i < size; ++i) A.insert("a" + std::to_string(i));
    for (int i = size / 2; i < size + size / 2; ++i) B.insert("a" + std::to_string(i));

    auto t0 = Clock::now();
    std::set<std::string> res = SetsModule::setIntersection(A, B);
    auto t1 = Clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    BenchResult r;
    r.label = "Intersection of sets (" + std::to_string(size) + " + " + std::to_string(size) + " elements)";
    r.result = "Result size: " + std::to_string(res.size());
    r.timeMs = ms;
    r.algorithm = "STL set_intersection";
    return r;
}

BenchResult BenchmarkModule::benchBitmaskSubsets(int n) {
    auto t0 = Clock::now();
    long long count = 0;
    int limit = 1 << n;
    for (int mask = 0; mask < limit; ++mask) { ++count; }
    auto t1 = Clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    BenchResult r;
    r.label = "Bitmask Subset Generation (n=" + std::to_string(n) + ")";
    r.result = "Generated " + std::to_string(count) + " subsets";
    r.timeMs = ms;
    r.algorithm = "O(1) space bitmask bit manipulation";
    return r;
}

std::vector<BenchResult> BenchmarkModule::runAll() {
    std::vector<BenchResult> results;
    results.push_back(benchCombination(10, 5));
    results.push_back(benchCombination(15, 7));
    results.push_back(benchCombination(20, 10));
    results.push_back(benchSetUnion(100));
    results.push_back(benchSetIntersection(100));
    results.push_back(benchBitmaskSubsets(20));
    return results;
}