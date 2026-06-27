#include "ProofModule.h"
#include "InductionModule.h"

std::vector<ProofStep> ProofModule::proveSubset(
    const std::set<std::string>& A,
    const std::set<std::string>& B,
    const std::string& labelA,
    const std::string& labelB)
{
    std::vector<ProofStep> steps;
    int line = 1;

    ProofStep goal;
    goal.lineNo = line++;
    goal.statement = "Goal: prove " + labelA + " subset of " + labelB;
    goal.justification = "Goal statement";
    goal.ok = true;
    steps.push_back(goal);

    bool allIn = true;
    for (std::set<std::string>::const_iterator it = A.begin(); it != A.end(); ++it) {
        bool inB = B.count(*it) > 0;
        if (!inB) allIn = false;
        ProofStep s;
        s.lineNo = line++;
        s.statement = "Element '" + *it + "' in " + labelA +
            (inB ? " and in " : " but NOT in ") + labelB;
        s.justification = "Direct check";
        s.ok = inB;
        steps.push_back(s);
    }

    ProofStep conc;
    conc.lineNo = line++;
    conc.statement = allIn
        ? "Therefore " + labelA + " subset of " + labelB + " (proven)"
        : "FAILED: counterexample found, " + labelA + " is NOT subset of " + labelB;
    conc.justification = "Conclusion";
    conc.ok = allIn;
    conc.bold = true;
    steps.push_back(conc);
    return steps;
}

std::vector<ProofStep> ProofModule::proveSetEquality(
    const std::set<std::string>& A,
    const std::set<std::string>& B)
{
    std::vector<ProofStep> steps;
    int line = 1;

    ProofStep title;
    title.lineNo = line++;
    title.statement = "PROOF: Set A = Set B";
    title.justification = "Theorem statement";
    title.ok = true;
    title.bold = true;
    steps.push_back(title);

    ProofStep strat;
    strat.lineNo = line++;
    strat.statement = "To prove: A subset B  AND  B subset A";
    strat.justification = "Strategy";
    strat.ok = true;
    steps.push_back(strat);

    // Part 1
    ProofStep p1hdr;
    p1hdr.lineNo = line++;
    p1hdr.statement = "--- Part 1: Proving A subset B ---";
    p1hdr.ok = true;
    steps.push_back(p1hdr);
    std::vector<ProofStep> sub1 = proveSubset(A, B, "A", "B");
    for (size_t i = 0; i < sub1.size(); ++i) { sub1[i].lineNo = line++; steps.push_back(sub1[i]); }

    // Part 2
    ProofStep p2hdr;
    p2hdr.lineNo = line++;
    p2hdr.statement = "--- Part 2: Proving B subset A ---";
    p2hdr.ok = true;
    steps.push_back(p2hdr);
    std::vector<ProofStep> sub2 = proveSubset(B, A, "B", "A");
    for (size_t i = 0; i < sub2.size(); ++i) { sub2[i].lineNo = line++; steps.push_back(sub2[i]); }

    bool eq = (A == B);
    ProofStep conc;
    conc.lineNo = line++;
    conc.statement = eq
        ? "Since A subset B and B subset A => A = B  (Q.E.D.)"
        : "Sets are NOT equal.";
    conc.justification = "Conclusion";
    conc.ok = eq;
    conc.bold = true;
    steps.push_back(conc);
    return steps;
}

std::vector<ProofStep> ProofModule::provePrerequisiteChain(
    const std::string& studentID,
    const std::string& courseCode,
    const UniversityDB& db)
{
    std::vector<ProofStep> steps;
    int line = 1;

    ProofStep title;
    title.lineNo = line++;
    title.statement = "PROOF: Student " + studentID + " may enroll in " + courseCode;
    title.ok = true; title.bold = true;
    steps.push_back(title);

    ProofStep base;
    base.lineNo = line++;
    base.statement = "Base case: direct prerequisites of " + courseCode;
    base.ok = true;
    steps.push_back(base);

    std::vector<InductionStep> indSteps = InductionModule::verifyPrerequisites(studentID, courseCode, db);
    for (size_t i = 0; i < indSteps.size(); ++i) {
        ProofStep s;
        s.lineNo = line++;
        s.statement = "Prerequisite '" + indSteps[i].course + "' [depth=" +
            std::to_string(indSteps[i].depth) + "] - " +
            (indSteps[i].completed ? "COMPLETED" : "NOT completed");
        s.justification = indSteps[i].depth == 0 ? "Base case" : "Inductive step";
        s.ok = indSteps[i].completed;
        steps.push_back(s);
    }

    bool ok = InductionModule::canEnroll(studentID, courseCode, db);
    ProofStep conc;
    conc.lineNo = line++;
    conc.statement = ok
        ? "All prerequisites satisfied => enrollment permitted  (Q.E.D.)"
        : "Some prerequisites missing => enrollment DENIED.";
    conc.ok = ok; conc.bold = true;
    steps.push_back(conc);
    return steps;
}