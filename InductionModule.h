#pragma once
#include "UniversityDB.h"
#include <vector>
#include <string>
#include <set>

struct InductionStep {
    std::string course;
    bool        completed;
    int         depth;
};

class InductionModule {
public:
    static std::vector<InductionStep> verifyPrerequisites(
        const std::string& studentID,
        const std::string& courseCode,
        const UniversityDB& db);

    static bool canEnroll(
        const std::string& studentID,
        const std::string& courseCode,
        const UniversityDB& db);

private:
    static void collectPrereqs(
        const std::string& code,
        const std::set<std::string>& completed,
        const UniversityDB& db,
        std::vector<InductionStep>& steps,
        std::set<std::string>& visited,
        int depth);
};