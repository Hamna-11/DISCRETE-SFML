#include "UniversityDB.h"
#include <string>
#include <vector>

struct ConsistencyIssue {
    std::string category;
    std::string description;
};

class ConsistencyChecker {
public:
    static std::vector<ConsistencyIssue> checkAll(const UniversityDB& db);

private:
    static void checkPrerequisites(const UniversityDB& db, std::vector<ConsistencyIssue>& issues);
    static void checkCapacity(const UniversityDB& db, std::vector<ConsistencyIssue>& issues);
    static void checkFacultyLoad(const UniversityDB& db, std::vector<ConsistencyIssue>& issues);
    static void checkCycleInPrereqs(const UniversityDB& db, std::vector<ConsistencyIssue>& issues);
};