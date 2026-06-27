#include "SetsModule.h"

std::set<std::string> SetsModule::setUnion(const std::set<std::string>& A, const std::set<std::string>& B) {
    std::set<std::string> result = A;
    result.insert(B.begin(), B.end());
    return result;
}

std::set<std::string> SetsModule::setIntersection(const std::set<std::string>& A, const std::set<std::string>& B) {
    std::set<std::string> result;
    for (std::set<std::string>::const_iterator it = A.begin(); it != A.end(); ++it)
        if (B.count(*it)) result.insert(*it);
    return result;
}

std::set<std::string> SetsModule::setDifference(const std::set<std::string>& A, const std::set<std::string>& B) {
    std::set<std::string> result;
    for (std::set<std::string>::const_iterator it = A.begin(); it != A.end(); ++it)
        if (!B.count(*it)) result.insert(*it);
    return result;
}

bool SetsModule::isSubset(const std::set<std::string>& A, const std::set<std::string>& B) {
    for (std::set<std::string>::const_iterator it = A.begin(); it != A.end(); ++it)
        if (!B.count(*it)) return false;
    return true;
}

std::vector<std::set<std::string>> SetsModule::powerSet(const std::set<std::string>& S) {
    std::vector<std::string> elems(S.begin(), S.end());
    int n = (int)elems.size();
    std::vector<std::set<std::string>> result;
    for (int mask = 0; mask < (1 << n); ++mask) {
        std::set<std::string> sub;
        for (int b = 0; b < n; ++b)
            if (mask & (1 << b)) sub.insert(elems[b]);
        result.push_back(sub);
    }
    return result;
}