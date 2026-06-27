#include "DataModels.h"
#include <map>
#include <vector>
#include <string>
#include <set>

class UniversityDB {
public:
    std::map<std::string, Course>  courses;
    std::map<std::string, Student> students;
    std::map<std::string, Faculty> faculty;
    std::map<std::string, Room>    rooms;

    std::vector<std::pair<std::string, std::string>> logicRules;
    std::set<std::string> facts;

    bool courseExists(const std::string& code) const {
        return courses.count(code) > 0;
    }
    bool studentExists(const std::string& id) const {
        return students.count(id) > 0;
    }
    bool facultyExists(const std::string& id) const {
        return faculty.count(id) > 0;
    }
    bool roomExists(const std::string& id) const {
        return rooms.count(id) > 0;
    }

    std::vector<std::string> getCourseIDs() const {
        std::vector<std::string> v;
        for (std::map<std::string, Course>::const_iterator it = courses.begin(); it != courses.end(); ++it)
            v.push_back(it->first);
        return v;
    }
    std::vector<std::string> getStudentIDs() const {
        std::vector<std::string> v;
        for (std::map<std::string, Student>::const_iterator it = students.begin(); it != students.end(); ++it)
            v.push_back(it->first);
        return v;
    }
    std::vector<std::string> getFacultyIDs() const {
        std::vector<std::string> v;
        for (std::map<std::string, Faculty>::const_iterator it = faculty.begin(); it != faculty.end(); ++it)
            v.push_back(it->first);
        return v;
    }
};