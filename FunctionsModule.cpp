#include "FunctionsModule.h"
#include <set>

std::map<std::string, std::string> FunctionsModule::courseFacultyMap(const UniversityDB& db) {
    std::map<std::string, std::string> f;
    for (std::map<std::string, Faculty>::const_iterator it = db.faculty.begin(); it != db.faculty.end(); ++it) {
        const Faculty& fac = it->second;
        for (size_t i = 0; i < fac.assignedCourses.size(); ++i)
            f[fac.assignedCourses[i]] = it->first;
    }
    return f;
}

std::map<std::string, std::string> FunctionsModule::studentCourseMap(const UniversityDB& db) {
    std::map<std::string, std::string> f;
    for (std::map<std::string, Student>::const_iterator it = db.students.begin(); it != db.students.end(); ++it) {
        const Student& st = it->second;
        if (!st.enrolledCourses.empty())
            f[it->first] = st.enrolledCourses[0];
    }
    return f;
}

FunctionProperties FunctionsModule::analyseFunction(
    const std::map<std::string, std::string>& f,
    const std::vector<std::string>& codomain)
{
    FunctionProperties p;
    std::set<std::string> imageSet;

    p.injective = true;
    for (std::map<std::string, std::string>::const_iterator it = f.begin(); it != f.end(); ++it) {
        if (imageSet.count(it->second)) { p.injective = false; }
        imageSet.insert(it->second);
    }

    p.surjective = true;
    for (size_t i = 0; i < codomain.size(); ++i)
        if (!imageSet.count(codomain[i])) { p.surjective = false; break; }

    p.bijective = p.injective && p.surjective;
    return p;
}

std::map<std::string, std::string> FunctionsModule::compose(
    const std::map<std::string, std::string>& f,
    const std::map<std::string, std::string>& g)
{
    std::map<std::string, std::string> result;
    for (std::map<std::string, std::string>::const_iterator it = f.begin(); it != f.end(); ++it) {
        std::map<std::string, std::string>::const_iterator git = g.find(it->second);
        if (git != g.end()) result[it->first] = git->second;
    }
    return result;
}

std::map<std::string, std::string> FunctionsModule::inverse(const std::map<std::string, std::string>& f) {
    std::map<std::string, std::string> inv;
    for (std::map<std::string, std::string>::const_iterator it = f.begin(); it != f.end(); ++it)
        inv[it->second] = it->first;
    return inv;
}