#pragma once
#include "UniversityDB.h"
#include <string>
#include <vector>
#include <set>

struct InferenceTrace {
    std::string step;
    bool        derived;
};

class LogicModule {
public:
    static void addRule(const std::string& antecedent, const std::string& consequent, UniversityDB& db);
    static void addFact(const std::string& fact, UniversityDB& db);
    static bool query(const std::string& target, const UniversityDB& db, std::vector<InferenceTrace>& trace);
    static std::vector<std::string> detectConflicts(const UniversityDB& db);
};