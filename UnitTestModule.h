#include <string>
#include <vector>

struct TestResult {
    std::string name;
    bool        passed;
    std::string detail;
    TestResult() : passed(false) {}
};

class UnitTestModule {
public:
    static std::vector<TestResult> runAllTests();

private:
    static std::vector<TestResult> testSets();
    static std::vector<TestResult> testCombinations();
    static std::vector<TestResult> testRelations();
    static std::vector<TestResult> testFunctions();
    static std::vector<TestResult> testLogic();
};