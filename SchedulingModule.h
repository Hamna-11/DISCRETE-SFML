#include "UniversityDB.h"
#include <vector>
#include <string>
#include <map>

class SchedulingModule {
public:
    static std::vector<std::string> topologicalSort(const UniversityDB& db);
    static std::vector<std::vector<std::string>> allValidSequences(const UniversityDB& db);

private:
    static bool dfsVisit(const std::string& node,
        const std::map<std::string, std::vector<std::string>>& adj,
        std::map<std::string, int>& color,
        std::vector<std::string>& order);
};