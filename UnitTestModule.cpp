#include "UnitTestModule.h"
#include "SetsModule.h"
#include "CombinationsModule.h"
#include "RelationsModule.h"
#include "FunctionsModule.h"
#include "LogicModule.h"
#include "UniversityDB.h"

static void addTest(std::vector<TestResult>& results,
    const std::string& name, bool cond, const std::string& detail = "") {
    TestResult t;
    t.name = name;
    t.passed = cond;
    t.detail = detail;
    results.push_back(t);
}

std::vector<TestResult> UnitTestModule::testSets() {
    std::vector<TestResult> results;
    std::set<std::string> A, B;
    A.insert("a"); A.insert("b"); A.insert("c");
    B.insert("b"); B.insert("c"); B.insert("d");

    addTest(results, "Union size correct",
        SetsModule::setUnion(A, B).size() == 4, "A union B = 4 elements");
    addTest(results, "Intersection size correct",
        SetsModule::setIntersection(A, B).size() == 2, "A intersect B = 2 elements");
    addTest(results, "Difference size correct",
        SetsModule::setDifference(A, B).size() == 1, "A - B = 1 element");
    addTest(results, "Subset check correct",
        !SetsModule::isSubset(A, B), "A is not subset of B");
    addTest(results, "Power set size correct (2^n)",
        SetsModule::powerSet(A).size() == 8, "PowerSet of {a,b,c} = 8 subsets");
    return results;
}

std::vector<TestResult> UnitTestModule::testCombinations() {
    std::vector<TestResult> results;
    addTest(results, "C(5,2) = 10", CombinationsModule::combination(5, 2) == 10, "");
    addTest(results, "C(10,3) = 120", CombinationsModule::combination(10, 3) == 120, "");
    addTest(results, "P(5,2) = 20", CombinationsModule::permutation(5, 2) == 20, "");
    addTest(results, "C(3,5) = 0 (invalid)", CombinationsModule::combination(3, 5) == 0, "r > n");
    return results;
}

std::vector<TestResult> UnitTestModule::testRelations() {
    std::vector<TestResult> results;
    std::set<std::string> dom;
    dom.insert("a"); dom.insert("b"); dom.insert("c");

    // Identity relation is reflexive, symmetric, transitive
    Relation R_id;
    R_id.insert(std::make_pair(std::string("a"), std::string("a")));
    R_id.insert(std::make_pair(std::string("b"), std::string("b")));
    R_id.insert(std::make_pair(std::string("c"), std::string("c")));
    RelationProperties p = RelationsModule::checkProperties(R_id, dom);

    addTest(results, "Reflexive property detected", p.reflexive, "Identity relation");
    addTest(results, "Symmetric property detected", p.symmetric, "Identity relation");
    addTest(results, "Transitive property detected", p.transitive, "Identity relation");
    return results;
}

std::vector<TestResult> UnitTestModule::testFunctions() {
    std::vector<TestResult> results;
    std::map<std::string, std::string> f_inj, f_non;
    f_inj["a"] = "x"; f_inj["b"] = "y"; f_inj["c"] = "z";
    f_non["a"] = "x"; f_non["b"] = "x"; f_non["c"] = "z";

    std::vector<std::string> cod;
    cod.push_back("x"); cod.push_back("y"); cod.push_back("z");

    FunctionProperties p_inj = FunctionsModule::analyseFunction(f_inj, cod);
    FunctionProperties p_non = FunctionsModule::analyseFunction(f_non, cod);

    addTest(results, "Injective function detected", p_inj.injective, "All distinct mappings");
    addTest(results, "Non-injective function detected", !p_non.injective, "a and b both map to x");
    return results;
}

std::vector<TestResult> UnitTestModule::testLogic() {
    std::vector<TestResult> results;
    UniversityDB db;
    LogicModule::addFact("A", db);
    LogicModule::addRule("A", "B", db);
    LogicModule::addRule("B", "C", db);

    std::vector<InferenceTrace> trace;
    bool r1 = LogicModule::query("B", db, trace);
    addTest(results, "Forward chaining infers B from A", r1, "A => B");

    bool r2 = LogicModule::query("C", db, trace);
    addTest(results, "Forward chaining infers C from A->B->C", r2, "A=>B=>C");

    bool r3 = LogicModule::query("D", db, trace);
    addTest(results, "Cannot infer D (not in knowledge base)", !r3, "D not derivable");
    return results;
}

std::vector<TestResult> UnitTestModule::runAllTests() {
    std::vector<TestResult> all;
    std::vector<TestResult> s1 = testSets();
    all.insert(all.end(), s1.begin(), s1.end());
    std::vector<TestResult> s2 = testCombinations();
    all.insert(all.end(), s2.begin(), s2.end());
    std::vector<TestResult> s3 = testRelations();
    all.insert(all.end(), s3.begin(), s3.end());
    std::vector<TestResult> s4 = testFunctions();
    all.insert(all.end(), s4.begin(), s4.end());
    std::vector<TestResult> s5 = testLogic();
    all.insert(all.end(), s5.begin(), s5.end());
    return all;
}