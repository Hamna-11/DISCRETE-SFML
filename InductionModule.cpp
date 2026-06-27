#include "InductionModule.h"

void InductionModule::collectPrereqs(const std::string& code,
    const std::set<std::string>& completed,
    const UniversityDB& db,
    std::vector<InductionStep>& steps,
    std::set<std::string>& visited,
    int depth)
{
    if (!db.courseExists(code)) return;
    const std::vector<std::string>& prereqs = db.courses.at(code).prerequisites;
    for (size_t i = 0; i < prereqs.size(); ++i) {
        const std::string& pre = prereqs[i];
        if (visited.count(pre)) continue;
        visited.insert(pre);
        InductionStep s;
        s.course = pre;
        s.completed = completed.count(pre) > 0;
        s.depth = depth;
        steps.push_back(s);
        collectPrereqs(pre, completed, db, steps, visited, depth + 1);
    }
}

std::vector<InductionStep> InductionModule::verifyPrerequisites(
    const std::string& studentID,
    const std::string& courseCode,
    const UniversityDB& db)
{
    std::vector<InductionStep> steps;
    if (!db.studentExists(studentID) || !db.courseExists(courseCode))
        return steps;

    const Student& st = db.students.at(studentID);
    std::set<std::string> completed(st.completedCourses.begin(), st.completedCourses.end());
    std::set<std::string> visited;
    collectPrereqs(courseCode, completed, db, steps, visited, 0);
    return steps;
}

bool InductionModule::canEnroll(const std::string& studentID,
    const std::string& courseCode,
    const UniversityDB& db)
{
    std::vector<InductionStep> steps = verifyPrerequisites(studentID, courseCode, db);
    for (size_t i = 0; i < steps.size(); ++i)
        if (!steps[i].completed) return false;
    return true;
}