#include "ConsistencyChecker.h"
#include "SchedulingModule.h"
#include <set>

void ConsistencyChecker::checkPrerequisites(const UniversityDB& db, std::vector<ConsistencyIssue>& issues) {
    for (std::map<std::string, Student>::const_iterator sit = db.students.begin(); sit != db.students.end(); ++sit) {
        const Student& st = sit->second;
        std::set<std::string> comp(st.completedCourses.begin(), st.completedCourses.end());
        for (size_t i = 0; i < st.enrolledCourses.size(); ++i) {
            const std::string& ec = st.enrolledCourses[i];
            if (!db.courseExists(ec)) {
                ConsistencyIssue iss;
                iss.category = "Missing Course";
                iss.description = "Student " + sit->first + " enrolled in unknown course " + ec;
                issues.push_back(iss);
                continue;
            }
            const std::vector<std::string>& prereqs = db.courses.at(ec).prerequisites;
            for (size_t j = 0; j < prereqs.size(); ++j) {
                if (!comp.count(prereqs[j])) {
                    ConsistencyIssue iss;
                    iss.category = "Prerequisite";
                    iss.description = "Student " + sit->first + " lacks prerequisite " +
                        prereqs[j] + " for " + ec;
                    issues.push_back(iss);
                }
            }
        }
    }
}

void ConsistencyChecker::checkCapacity(const UniversityDB& db, std::vector<ConsistencyIssue>& issues) {
    std::map<std::string, int> counts;
    for (std::map<std::string, Student>::const_iterator sit = db.students.begin(); sit != db.students.end(); ++sit) {
        const Student& st = sit->second;
        for (size_t i = 0; i < st.enrolledCourses.size(); ++i)
            counts[st.enrolledCourses[i]]++;
    }
    for (std::map<std::string, Course>::const_iterator cit = db.courses.begin(); cit != db.courses.end(); ++cit) {
        const std::string& code = cit->first;
        if (counts[code] > cit->second.capacity) {
            ConsistencyIssue iss;
            iss.category = "Capacity Overflow";
            iss.description = "Course " + code + " has " + std::to_string(counts[code]) +
                " students but capacity is " + std::to_string(cit->second.capacity);
            issues.push_back(iss);
        }
    }
}

void ConsistencyChecker::checkFacultyLoad(const UniversityDB& db, std::vector<ConsistencyIssue>& issues) {
    for (std::map<std::string, Faculty>::const_iterator fit = db.faculty.begin(); fit != db.faculty.end(); ++fit) {
        const Faculty& f = fit->second;
        if ((int)f.assignedCourses.size() > f.maxLoad) {
            ConsistencyIssue iss;
            iss.category = "Faculty Overload";
            iss.description = "Faculty " + fit->first + " assigned " +
                std::to_string(f.assignedCourses.size()) +
                " courses but max is " + std::to_string(f.maxLoad);
            issues.push_back(iss);
        }
    }
}

void ConsistencyChecker::checkCycleInPrereqs(const UniversityDB& db, std::vector<ConsistencyIssue>& issues) {
    std::vector<std::string> seq = SchedulingModule::topologicalSort(db);
    if (seq.empty() && !db.courses.empty()) {
        ConsistencyIssue iss;
        iss.category = "Cycle";
        iss.description = "Prerequisite graph contains a cycle!";
        issues.push_back(iss);
    }
}

std::vector<ConsistencyIssue> ConsistencyChecker::checkAll(const UniversityDB& db) {
    std::vector<ConsistencyIssue> issues;
    checkPrerequisites(db, issues);
    checkCapacity(db, issues);
    checkFacultyLoad(db, issues);
    checkCycleInPrereqs(db, issues);
    return issues;
}