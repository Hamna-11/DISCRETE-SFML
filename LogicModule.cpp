#include "LogicModule.h"

void LogicModule::addRule(const std::string& ant, const std::string& con, UniversityDB& db) {
    db.logicRules.push_back(std::make_pair(ant, con));
}

void LogicModule::addFact(const std::string& fact, UniversityDB& db) {
    db.facts.insert(fact);
}

bool LogicModule::query(const std::string& target,
    const UniversityDB& db,
    std::vector<InferenceTrace>& trace)
{
    std::set<std::string> derived = db.facts;
    trace.clear();

    for (std::set<std::string>::const_iterator it = db.facts.begin(); it != db.facts.end(); ++it) {
        InferenceTrace t;
        t.step = "FACT: " + *it;
        t.derived = true;
        trace.push_back(t);
    }

    bool changed = true;
    while (changed) {
        changed = false;
        for (size_t i = 0; i < db.logicRules.size(); ++i) {
            const std::string& ant = db.logicRules[i].first;
            const std::string& con = db.logicRules[i].second;
            if (derived.count(ant) && !derived.count(con)) {
                derived.insert(con);
                InferenceTrace t;
                t.step = "APPLY: " + ant + " => " + con;
                t.derived = true;
                trace.push_back(t);
                changed = true;
            }
        }
    }

    InferenceTrace t;
    if (derived.count(target)) {
        t.step = "QUERY '" + target + "' found in derived facts.";
        t.derived = true;
        trace.push_back(t);
        return true;
    }
    t.step = "QUERY '" + target + "' NOT found.";
    t.derived = false;
    trace.push_back(t);
    return false;
}

std::vector<std::string> LogicModule::detectConflicts(const UniversityDB& db) {
    std::vector<std::string> conflicts;
    for (std::map<std::string, Student>::const_iterator sit = db.students.begin(); sit != db.students.end(); ++sit) {
        const Student& st = sit->second;
        std::set<std::string> comp(st.completedCourses.begin(), st.completedCourses.end());
        for (size_t i = 0; i < st.enrolledCourses.size(); ++i) {
            const std::string& ec = st.enrolledCourses[i];
            if (!db.courseExists(ec)) continue;
            const std::vector<std::string>& prereqs = db.courses.at(ec).prerequisites;
            for (size_t j = 0; j < prereqs.size(); ++j) {
                if (!comp.count(prereqs[j]))
                    conflicts.push_back("Student " + sit->first +
                        " enrolled in " + ec +
                        " but missing prerequisite " + prereqs[j]);
            }
        }
    }
    return conflicts;
}