#pragma once
#include "UniversityDB.h"
#include <string>
#include <vector>
#include <set>

struct ProofStep {
    int         lineNo;
    std::string statement;
    std::string justification;
    bool        ok;
    bool        bold;
    ProofStep() : lineNo(0), ok(true), bold(false) {}
};

class ProofModule {
public:
    static std::vector<ProofStep> proveSetEquality(
        const std::set<std::string>& A,
        const std::set<std::string>& B);

    static std::vector<ProofStep> proveSubset(
        const std::set<std::string>& A,
        const std::set<std::string>& B,
        const std::string& labelA,
        const std::string& labelB);

    static std::vector<ProofStep> provePrerequisiteChain(
        const std::string& studentID,
        const std::string& courseCode,
        const UniversityDB& db);
};