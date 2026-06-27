#include <set>
#include <vector>
#include <string>

class SetsModule {
public:
    static std::set<std::string> setUnion(const std::set<std::string>& A, const std::set<std::string>& B);
    static std::set<std::string> setIntersection(const std::set<std::string>& A, const std::set<std::string>& B);
    static std::set<std::string> setDifference(const std::set<std::string>& A, const std::set<std::string>& B);
    static bool                  isSubset(const std::set<std::string>& A, const std::set<std::string>& B);
    static std::vector<std::set<std::string>> powerSet(const std::set<std::string>& S);
};