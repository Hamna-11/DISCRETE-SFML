#pragma once
#include <string>
#include <vector>
#include <set>

struct Course {
    std::string code;
    std::string name;
    int credits;
    int capacity;
    std::vector<std::string> prerequisites;
    std::string facultyID;
    std::string roomID;

    Course() : credits(0), capacity(0) {}
};

struct Student {
    std::string id;
    std::string name;
    int semester;
    std::vector<std::string> completedCourses;
    std::vector<std::string> enrolledCourses;

    Student() : semester(1) {}
};

struct Faculty {
    std::string id;
    std::string name;
    int maxLoad;
    std::vector<std::string> assignedCourses;

    Faculty() : maxLoad(3) {}
};

struct Room {
    std::string id;
    int capacity;
    bool isLab;
    std::vector<std::string> assignedCourses;

    Room() : capacity(0), isLab(false) {}
};