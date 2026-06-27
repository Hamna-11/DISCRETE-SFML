#pragma once
#include "UniversityDB.h"
#include <map>
#include <string>
#include <vector>

struct FunctionProperties {
    bool injective;
    bool surjective;
    bool bijective;
    FunctionProperties() : injective(false), surjective(false), bijective(false) {}
};

class FunctionsModule {
public:
    static std::map<std::string, std::string> courseFacultyMap(const UniversityDB& db);
    static std::map<std::string, std::string> studentCourseMap(const UniversityDB& db);
    static FunctionProperties analyseFunction(
        const std::map<std::string, std::string>& f,
        const std::vector<std::string>& codomain);
    static std::map<std::string, std::string> compose(
        const std::map<std::string, std::string>& f,
        const std::map<std::string, std::string>& g);
    static std::map<std::string, std::string> inverse(
        const std::map<std::string, std::string>& f);
};