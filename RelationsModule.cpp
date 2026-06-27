#include "RelationsModule.h"

Relation RelationsModule::buildStudentCourseRelation(const UniversityDB& db) {
    Relation R;
    for (std::map<std::string, Student>::const_iterator sit = db.students.begin(); sit != db.students.end(); ++sit) {
        const Student& st = sit->second;
        for (size_t i = 0; i < st.enrolledCourses.size(); ++i)
            R.insert(std::make_pair(sit->first, st.enrolledCourses[i]));
    }
    return R;
}

Relation RelationsModule::buildFacultyCourseRelation(const UniversityDB& db) {
    Relation R;
    for (std::map<std::string, Faculty>::const_iterator fit = db.faculty.begin(); fit != db.faculty.end(); ++fit) {
        const Faculty& f = fit->second;
        for (size_t i = 0; i < f.assignedCourses.size(); ++i)
            R.insert(std::make_pair(fit->first, f.assignedCourses[i]));
    }
    return R;
}

Relation RelationsModule::buildPrerequisiteRelation(const UniversityDB& db) {
    Relation R;
    for (std::map<std::string, Course>::const_iterator cit = db.courses.begin(); cit != db.courses.end(); ++cit) {
        const Course& c = cit->second;
        for (size_t i = 0; i < c.prerequisites.size(); ++i)
            R.insert(std::make_pair(c.prerequisites[i], cit->first));
    }
    return R;
}

RelationProperties RelationsModule::checkProperties(const Relation& R, const std::set<std::string>& domain) {
    RelationProperties p;

    // Reflexive
    p.reflexive = true;
    for (std::set<std::string>::const_iterator it = domain.begin(); it != domain.end(); ++it)
        if (!R.count(std::make_pair(*it, *it))) { p.reflexive = false; break; }

    // Symmetric
    p.symmetric = true;
    for (Relation::const_iterator it = R.begin(); it != R.end(); ++it)
        if (!R.count(std::make_pair(it->second, it->first))) { p.symmetric = false; break; }

    // Antisymmetric
    p.antisymmetric = true;
    for (Relation::const_iterator it = R.begin(); it != R.end(); ++it)
        if (it->first != it->second && R.count(std::make_pair(it->second, it->first)))
        {
            p.antisymmetric = false; break;
        }

    // Transitive
    p.transitive = true;
    for (Relation::const_iterator it1 = R.begin(); it1 != R.end() && p.transitive; ++it1) {
        for (Relation::const_iterator it2 = R.begin(); it2 != R.end() && p.transitive; ++it2) {
            if (it1->second == it2->first && !R.count(std::make_pair(it1->first, it2->second)))
                p.transitive = false;
        }
    }

    p.equivalence = p.reflexive && p.symmetric && p.transitive;
    p.partialOrder = p.reflexive && p.antisymmetric && p.transitive;
    return p;
}

Relation RelationsModule::transitiveClosure(const Relation& R) {
    Relation closure = R;
    bool changed = true;
    while (changed) {
        changed = false;
        std::vector<std::pair<std::string, std::string>> newPairs;
        for (Relation::const_iterator it1 = closure.begin(); it1 != closure.end(); ++it1) {
            for (Relation::const_iterator it2 = closure.begin(); it2 != closure.end(); ++it2) {
                if (it1->second == it2->first) {
                    std::pair<std::string, std::string> np(it1->first, it2->second);
                    if (!closure.count(np)) newPairs.push_back(np);
                }
            }
        }
        for (size_t i = 0; i < newPairs.size(); ++i) {
            closure.insert(newPairs[i]);
            changed = true;
        }
    }
    return closure;
}

Relation RelationsModule::compose(const Relation& R1, const Relation& R2) {
    Relation result;
    for (Relation::const_iterator it1 = R1.begin(); it1 != R1.end(); ++it1)
        for (Relation::const_iterator it2 = R2.begin(); it2 != R2.end(); ++it2)
            if (it1->second == it2->first)
                result.insert(std::make_pair(it1->first, it2->second));
    return result;
}

std::vector<std::vector<int>> RelationsModule::toMatrix(const Relation& R, const std::vector<std::string>& domain) {
    int n = (int)domain.size();
    std::map<std::string, int> idx;
    for (int i = 0; i < n; ++i) idx[domain[i]] = i;

    std::vector<std::vector<int>> mat(n, std::vector<int>(n, 0));
    for (Relation::const_iterator it = R.begin(); it != R.end(); ++it) {
        if (idx.count(it->first) && idx.count(it->second))
            mat[idx[it->first]][idx[it->second]] = 1;
    }
    return mat;
}