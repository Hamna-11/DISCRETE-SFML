#include <vector>
#include <string>
#include <map>

class CombinationsModule {
public:
    static long long combination(int n, int r);
    static long long permutation(int n, int r);
    static std::vector<std::vector<std::string>> allCombinations(const std::vector<std::string>& items, int r);
    static std::vector<std::vector<std::string>> sequentialGroups(const std::vector<std::string>& items, int groupSize);

private:
    static std::map<std::pair<int, int>, long long> memo;
};