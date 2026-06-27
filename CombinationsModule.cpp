#include "CombinationsModule.h"

std::map<std::pair<int, int>, long long> CombinationsModule::memo;

long long CombinationsModule::combination(int n, int r) {
    if (r < 0 || r > n) return 0;
    if (r == 0 || r == n) return 1;
    std::pair<int, int> key(n, r);
    std::map<std::pair<int, int>, long long>::iterator it = memo.find(key);
    if (it != memo.end()) return it->second;
    long long val = combination(n - 1, r - 1) + combination(n - 1, r);
    memo[key] = val;
    return val;
}

long long CombinationsModule::permutation(int n, int r) {
    if (r < 0 || r > n) return 0;
    long long result = 1;
    for (int i = 0; i < r; ++i) result *= (n - i);
    return result;
}

// Count set bits without __builtin_popcount
static int countBits(int x) {
    int c = 0;
    while (x) { c += (x & 1); x >>= 1; }
    return c;
}

std::vector<std::vector<std::string>>
CombinationsModule::allCombinations(const std::vector<std::string>& items, int r) {
    std::vector<std::vector<std::string>> result;
    int n = (int)items.size();
    if (r > n || r < 0) return result;

    for (int mask = 0; mask < (1 << n); ++mask) {
        if (countBits(mask) == r) {
            std::vector<std::string> grp;
            for (int b = 0; b < n; ++b)
                if (mask & (1 << b)) grp.push_back(items[b]);
            result.push_back(grp);
        }
    }
    return result;
}

std::vector<std::vector<std::string>>
CombinationsModule::sequentialGroups(const std::vector<std::string>& items, int groupSize) {
    std::vector<std::vector<std::string>> groups;
    std::vector<std::string> cur;
    for (size_t i = 0; i < items.size(); ++i) {
        cur.push_back(items[i]);
        if ((int)cur.size() == groupSize) {
            groups.push_back(cur);
            cur.clear();
        }
    }
    if (!cur.empty()) groups.push_back(cur);
    return groups;
}