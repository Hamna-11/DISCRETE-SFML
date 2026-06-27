// ================================================================
//  FAST University - Discrete Structures System
//  Single-file C++14 + SFML GUI  |  Fullscreen, polished UI
// ================================================================
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <iterator>
#include <cmath>

// ================================================================
//  SECTION 1: DATA MODELS
// ================================================================
struct Course {
    std::string code, name, facultyID, roomID;
    int credits, capacity;
    std::vector<std::string> prerequisites;
    Course() : credits(0), capacity(0) {}
};
struct Student {
    std::string id, name;
    int semester;
    std::vector<std::string> completedCourses, enrolledCourses;
    Student() : semester(1) {}
};
struct Faculty {
    std::string id, name;
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

// ================================================================
//  SECTION 2: UNIVERSITY DATABASE
// ================================================================
struct UniversityDB {
    std::map<std::string,Course>   courses;
    std::map<std::string,Student>  students;
    std::map<std::string,Faculty>  faculty;
    std::map<std::string,Room>     rooms;
    std::vector<std::pair<std::string,std::string>> logicRules;
    std::set<std::string> facts;

    bool courseExists (const std::string& c) const { return courses.count(c)  > 0; }
    bool studentExists(const std::string& s) const { return students.count(s) > 0; }

    std::vector<std::string> getStudentIDs() const {
        std::vector<std::string> v;
        for (std::map<std::string,Student>::const_iterator it = students.begin(); it != students.end(); ++it)
            v.push_back(it->first);
        return v;
    }
    std::vector<std::string> getFacultyIDs() const {
        std::vector<std::string> v;
        for (std::map<std::string,Faculty>::const_iterator it = faculty.begin(); it != faculty.end(); ++it)
            v.push_back(it->first);
        return v;
    }
};

// ================================================================
//  SECTION 3: SCHEDULING MODULE (Topological Sort)
// ================================================================
static bool topoVisit(const std::string& node,
    const std::map<std::string,std::vector<std::string>>& adj,
    std::map<std::string,int>& color,
    std::vector<std::string>& order)
{
    color[node] = 1;
    std::map<std::string,std::vector<std::string>>::const_iterator it = adj.find(node);
    if (it != adj.end()) {
        for (size_t i = 0; i < it->second.size(); ++i) {
            const std::string& nb = it->second[i];
            if (color[nb] == 1) return false;
            if (color[nb] == 0)
                if (!topoVisit(nb, adj, color, order)) return false;
        }
    }
    color[node] = 2;
    order.push_back(node);
    return true;
}

static std::vector<std::string> topoSort(const UniversityDB& db) {
    std::map<std::string,std::vector<std::string>> adj;
    for (std::map<std::string,Course>::const_iterator it = db.courses.begin(); it != db.courses.end(); ++it) {
        const Course& c = it->second;
        for (size_t i = 0; i < c.prerequisites.size(); ++i)
            adj[c.prerequisites[i]].push_back(it->first);
    }
    std::map<std::string,int> color;
    for (std::map<std::string,Course>::const_iterator it = db.courses.begin(); it != db.courses.end(); ++it)
        color[it->first] = 0;
    std::vector<std::string> order;
    for (std::map<std::string,Course>::const_iterator it = db.courses.begin(); it != db.courses.end(); ++it)
        if (color[it->first] == 0)
            if (!topoVisit(it->first, adj, color, order))
                return std::vector<std::string>();
    return order;
}

// ================================================================
//  SECTION 4: COMBINATIONS MODULE
// ================================================================
static std::map<std::pair<int,int>,long long> combMemo;

static long long comb(int n, int r) {
    if (r < 0 || r > n) return 0;
    if (r == 0 || r == n) return 1;
    std::pair<int,int> key(n,r);
    std::map<std::pair<int,int>,long long>::iterator it = combMemo.find(key);
    if (it != combMemo.end()) return it->second;
    long long v = comb(n-1,r-1) + comb(n-1,r);
    combMemo[key] = v;
    return v;
}
static long long perm(int n, int r) {
    if (r < 0 || r > n) return 0;
    long long res = 1;
    for (int i = 0; i < r; ++i) res *= (n - i);
    return res;
}
static int countBits(int x) { int c=0; while(x){c+=(x&1);x>>=1;} return c; }

static std::vector<std::vector<std::string>> allCombinations(const std::vector<std::string>& items, int r) {
    std::vector<std::vector<std::string>> result;
    int n = (int)items.size();
    if (r > n || r < 0) return result;
    for (int mask = 0; mask < (1 << n); ++mask) {
        if (countBits(mask) == r) {
            std::vector<std::string> grp;
            for (int b = 0; b < n; ++b)
                if (mask & (1 << b)) grp.push_back(items[b]);
            result.push_back(grp);
        }
    }
    return result;
}
static std::vector<std::vector<std::string>> seqGroups(const std::vector<std::string>& items, int gs) {
    std::vector<std::vector<std::string>> groups;
    std::vector<std::string> cur;
    for (size_t i = 0; i < items.size(); ++i) {
        cur.push_back(items[i]);
        if ((int)cur.size() == gs) { groups.push_back(cur); cur.clear(); }
    }
    if (!cur.empty()) groups.push_back(cur);
    return groups;
}

// ================================================================
//  SECTION 5: INDUCTION MODULE
// ================================================================
struct InductionStep { std::string course; bool completed; int depth; };

static void collectPrereqs(const std::string& code,
    const std::set<std::string>& completed,
    const UniversityDB& db,
    std::vector<InductionStep>& steps,
    std::set<std::string>& visited, int depth)
{
    if (!db.courseExists(code)) return;
    const std::vector<std::string>& prereqs = db.courses.at(code).prerequisites;
    for (size_t i = 0; i < prereqs.size(); ++i) {
        const std::string& pre = prereqs[i];
        if (visited.count(pre)) continue;
        visited.insert(pre);
        InductionStep s; s.course = pre; s.completed = completed.count(pre)>0; s.depth = depth;
        steps.push_back(s);
        collectPrereqs(pre, completed, db, steps, visited, depth+1);
    }
}
static std::vector<InductionStep> verifyPrereqs(const std::string& sid, const std::string& code, const UniversityDB& db) {
    std::vector<InductionStep> steps;
    if (!db.studentExists(sid) || !db.courseExists(code)) return steps;
    const Student& st = db.students.at(sid);
    std::set<std::string> comp(st.completedCourses.begin(), st.completedCourses.end());
    std::set<std::string> visited;
    collectPrereqs(code, comp, db, steps, visited, 0);
    return steps;
}
static bool canEnroll(const std::string& sid, const std::string& code, const UniversityDB& db) {
    std::vector<InductionStep> s = verifyPrereqs(sid, code, db);
    for (size_t i = 0; i < s.size(); ++i) if (!s[i].completed) return false;
    return true;
}

// ================================================================
//  SECTION 6: LOGIC & INFERENCE ENGINE
// ================================================================
struct InferenceTrace { std::string step; bool derived; };

static void addRule(const std::string& ant, const std::string& con, UniversityDB& db) {
    db.logicRules.push_back(std::make_pair(ant, con));
}
static void addFact(const std::string& fact, UniversityDB& db) { db.facts.insert(fact); }

static bool queryLogic(const std::string& target, const UniversityDB& db, std::vector<InferenceTrace>& trace) {
    std::set<std::string> derived = db.facts;
    trace.clear();
    for (std::set<std::string>::const_iterator it = db.facts.begin(); it != db.facts.end(); ++it) {
        InferenceTrace t; t.step = "FACT: " + *it; t.derived = true; trace.push_back(t);
    }
    bool changed = true;
    while (changed) {
        changed = false;
        for (size_t i = 0; i < db.logicRules.size(); ++i) {
            const std::string& ant = db.logicRules[i].first;
            const std::string& con = db.logicRules[i].second;
            if (derived.count(ant) && !derived.count(con)) {
                derived.insert(con);
                InferenceTrace t; t.step = "APPLY: " + ant + " => " + con; t.derived = true;
                trace.push_back(t); changed = true;
            }
        }
    }
    InferenceTrace t;
    if (derived.count(target)) {
        t.step = "QUERY '" + target + "' found in derived facts."; t.derived = true;
        trace.push_back(t); return true;
    }
    t.step = "QUERY '" + target + "' NOT found."; t.derived = false;
    trace.push_back(t); return false;
}

// ================================================================
//  SECTION 7: SET OPERATIONS MODULE
// ================================================================
static std::set<std::string> setUnion(const std::set<std::string>& A, const std::set<std::string>& B) {
    std::set<std::string> r = A; r.insert(B.begin(), B.end()); return r;
}
static std::set<std::string> setIntersect(const std::set<std::string>& A, const std::set<std::string>& B) {
    std::set<std::string> r;
    for (std::set<std::string>::const_iterator it = A.begin(); it != A.end(); ++it)
        if (B.count(*it)) r.insert(*it);
    return r;
}
static std::set<std::string> setDiff(const std::set<std::string>& A, const std::set<std::string>& B) {
    std::set<std::string> r;
    for (std::set<std::string>::const_iterator it = A.begin(); it != A.end(); ++it)
        if (!B.count(*it)) r.insert(*it);
    return r;
}
static bool isSubset(const std::set<std::string>& A, const std::set<std::string>& B) {
    for (std::set<std::string>::const_iterator it = A.begin(); it != A.end(); ++it)
        if (!B.count(*it)) return false;
    return true;
}
static std::vector<std::set<std::string>> powerSet(const std::set<std::string>& S) {
    std::vector<std::string> elems(S.begin(), S.end());
    int n = (int)elems.size();
    std::vector<std::set<std::string>> result;
    for (int mask = 0; mask < (1 << n); ++mask) {
        std::set<std::string> sub;
        for (int b = 0; b < n; ++b) if (mask & (1 << b)) sub.insert(elems[b]);
        result.push_back(sub);
    }
    return result;
}

// ================================================================
//  SECTION 8: RELATIONS MODULE
// ================================================================
typedef std::set<std::pair<std::string,std::string>> Relation;

struct RelProps { bool refl, symm, trans, anti, equiv, partial; RelProps():refl(false),symm(false),trans(false),anti(false),equiv(false),partial(false){} };

static Relation buildSCRelation(const UniversityDB& db) {
    Relation R;
    for (std::map<std::string,Student>::const_iterator it = db.students.begin(); it != db.students.end(); ++it) {
        const Student& st = it->second;
        for (size_t i = 0; i < st.enrolledCourses.size(); ++i)
            R.insert(std::make_pair(it->first, st.enrolledCourses[i]));
    }
    return R;
}
static RelProps checkRelProps(const Relation& R, const std::set<std::string>& dom) {
    RelProps p;
    p.refl = true;
    for (std::set<std::string>::const_iterator it = dom.begin(); it != dom.end(); ++it)
        if (!R.count(std::make_pair(*it,*it))) { p.refl = false; break; }
    p.symm = true;
    for (Relation::const_iterator it = R.begin(); it != R.end(); ++it)
        if (!R.count(std::make_pair(it->second,it->first))) { p.symm = false; break; }
    p.anti = true;
    for (Relation::const_iterator it = R.begin(); it != R.end(); ++it)
        if (it->first != it->second && R.count(std::make_pair(it->second,it->first))) { p.anti = false; break; }
    p.trans = true;
    for (Relation::const_iterator a = R.begin(); a != R.end() && p.trans; ++a)
        for (Relation::const_iterator b = R.begin(); b != R.end() && p.trans; ++b)
            if (a->second == b->first && !R.count(std::make_pair(a->first,b->second))) p.trans = false;
    p.equiv   = p.refl && p.symm  && p.trans;
    p.partial = p.refl && p.anti  && p.trans;
    return p;
}
static std::vector<std::vector<int>> relMatrix(const Relation& R, const std::vector<std::string>& dom) {
    int n = (int)dom.size();
    std::map<std::string,int> idx;
    for (int i = 0; i < n; ++i) idx[dom[i]] = i;
    std::vector<std::vector<int>> mat(n, std::vector<int>(n, 0));
    for (Relation::const_iterator it = R.begin(); it != R.end(); ++it)
        if (idx.count(it->first) && idx.count(it->second))
            mat[idx[it->first]][idx[it->second]] = 1;
    return mat;
}

// ================================================================
//  SECTION 9: FUNCTIONS MODULE
// ================================================================
struct FuncProps { bool injective, surjective, bijective; FuncProps():injective(false),surjective(false),bijective(false){} };

static std::map<std::string,std::string> courseFacultyMap(const UniversityDB& db) {
    std::map<std::string,std::string> f;
    for (std::map<std::string,Faculty>::const_iterator it = db.faculty.begin(); it != db.faculty.end(); ++it)
        for (size_t i = 0; i < it->second.assignedCourses.size(); ++i)
            f[it->second.assignedCourses[i]] = it->first;
    return f;
}
static FuncProps analyseFunc(const std::map<std::string,std::string>& f, const std::vector<std::string>& codomain) {
    FuncProps p;
    std::set<std::string> img;
    p.injective = true;
    for (std::map<std::string,std::string>::const_iterator it = f.begin(); it != f.end(); ++it) {
        if (img.count(it->second)) p.injective = false;
        img.insert(it->second);
    }
    p.surjective = true;
    for (size_t i = 0; i < codomain.size(); ++i) if (!img.count(codomain[i])) { p.surjective = false; break; }
    p.bijective = p.injective && p.surjective;
    return p;
}

// ================================================================
//  SECTION 10: PROOF MODULE
// ================================================================
struct ProofStep { int line; std::string stmt, just; bool ok, bold; ProofStep():line(0),ok(true),bold(false){} };

static std::vector<ProofStep> proveSubset(const std::set<std::string>& A, const std::set<std::string>& B,
    const std::string& la, const std::string& lb)
{
    std::vector<ProofStep> steps; int ln = 1;
    ProofStep g; g.line=ln++; g.stmt="Goal: prove "+la+" subset of "+lb; g.just="Goal statement"; g.ok=true; steps.push_back(g);
    bool allIn = true;
    for (std::set<std::string>::const_iterator it = A.begin(); it != A.end(); ++it) {
        bool inB = B.count(*it) > 0; if (!inB) allIn = false;
        ProofStep s; s.line=ln++; s.stmt="Element '"+*it+"' in "+la+(inB?" and in ":" but NOT in ")+lb;
        s.just="Direct check"; s.ok=inB; steps.push_back(s);
    }
    ProofStep c; c.line=ln++; c.stmt=allIn?"Therefore "+la+" subset "+lb+" (proven)":"FAILED: "+la+" is NOT subset of "+lb;
    c.just="Conclusion"; c.ok=allIn; c.bold=true; steps.push_back(c);
    return steps;
}
static std::vector<ProofStep> proveSetEquality(const std::set<std::string>& A, const std::set<std::string>& B) {
    std::vector<ProofStep> steps; int ln = 1;
    ProofStep t; t.line=ln++; t.stmt="PROOF: Set A = Set B"; t.just="Theorem"; t.ok=true; t.bold=true; steps.push_back(t);
    ProofStep s; s.line=ln++; s.stmt="To prove: A subset B  AND  B subset A"; s.ok=true; steps.push_back(s);
    ProofStep h1; h1.line=ln++; h1.stmt="--- Part 1: Proving A subset B ---"; h1.ok=true; steps.push_back(h1);
    std::vector<ProofStep> sub1 = proveSubset(A,B,"A","B");
    for (size_t i=0;i<sub1.size();++i){sub1[i].line=ln++;steps.push_back(sub1[i]);}
    ProofStep h2; h2.line=ln++; h2.stmt="--- Part 2: Proving B subset A ---"; h2.ok=true; steps.push_back(h2);
    std::vector<ProofStep> sub2 = proveSubset(B,A,"B","A");
    for (size_t i=0;i<sub2.size();++i){sub2[i].line=ln++;steps.push_back(sub2[i]);}
    bool eq = (A==B);
    ProofStep c; c.line=ln++; c.stmt=eq?"Since A subset B and B subset A => A = B  (Q.E.D.)":"Sets are NOT equal.";
    c.ok=eq; c.bold=true; steps.push_back(c);
    return steps;
}

// ================================================================
//  SECTION 11: CONSISTENCY CHECKER
// ================================================================
struct CIssue { std::string cat, desc; };

static std::vector<CIssue> checkConsistency(const UniversityDB& db) {
    std::vector<CIssue> issues;
    for (std::map<std::string,Student>::const_iterator sit = db.students.begin(); sit != db.students.end(); ++sit) {
        const Student& st = sit->second;
        std::set<std::string> comp(st.completedCourses.begin(), st.completedCourses.end());
        for (size_t i = 0; i < st.enrolledCourses.size(); ++i) {
            const std::string& ec = st.enrolledCourses[i];
            if (!db.courseExists(ec)) {
                CIssue iss; iss.cat="Missing Course"; iss.desc="Student "+sit->first+" enrolled in unknown course "+ec;
                issues.push_back(iss); continue;
            }
            const std::vector<std::string>& prereqs = db.courses.at(ec).prerequisites;
            for (size_t j = 0; j < prereqs.size(); ++j) {
                if (!comp.count(prereqs[j])) {
                    CIssue iss; iss.cat="Prerequisite";
                    iss.desc="Student "+sit->first+" lacks prerequisite "+prereqs[j]+" for "+ec;
                    issues.push_back(iss);
                }
            }
        }
    }
    for (std::map<std::string,Faculty>::const_iterator fit = db.faculty.begin(); fit != db.faculty.end(); ++fit) {
        const Faculty& f = fit->second;
        if ((int)f.assignedCourses.size() > f.maxLoad) {
            CIssue iss; iss.cat="Faculty Overload";
            iss.desc="Faculty "+fit->first+" assigned "+std::to_string(f.assignedCourses.size())+
                     " courses but max is "+std::to_string(f.maxLoad);
            issues.push_back(iss);
        }
    }
    std::vector<std::string> seq = topoSort(db);
    if (seq.empty() && !db.courses.empty()) {
        CIssue iss; iss.cat="Cycle"; iss.desc="Prerequisite graph contains a cycle!";
        issues.push_back(iss);
    }
    return issues;
}

// ================================================================
//  SECTION 12: BENCHMARK MODULE
// ================================================================
struct BenchResult { std::string label, result, algorithm; double timeMs; BenchResult():timeMs(0){} };

static std::vector<BenchResult> runBenchmarks() {
    std::vector<BenchResult> res;
    typedef std::chrono::high_resolution_clock Clock;
    int ns[]={10,15,20}; int rs[]={5,7,10};
    for (int i = 0; i < 3; ++i) {
        auto t0 = Clock::now(); long long v = comb(ns[i],rs[i]); auto t1 = Clock::now();
        BenchResult r;
        r.label="C("+std::to_string(ns[i])+","+std::to_string(rs[i])+")";
        r.result=std::to_string(v);
        r.timeMs=std::chrono::duration<double,std::milli>(t1-t0).count();
        r.algorithm="Dynamic Programming with Memoization";
        res.push_back(r);
    }
    {
        std::set<std::string> A,B;
        for (int i=0;i<100;++i) A.insert("a"+std::to_string(i));
        for (int i=50;i<150;++i) B.insert("a"+std::to_string(i));
        auto t0=Clock::now(); std::set<std::string> u=setUnion(A,B); auto t1=Clock::now();
        BenchResult r; r.label="Union(100+100)"; r.result="Size: "+std::to_string(u.size());
        r.timeMs=std::chrono::duration<double,std::milli>(t1-t0).count(); r.algorithm="STL set insert";
        res.push_back(r);
        auto t2=Clock::now(); std::set<std::string> in=setIntersect(A,B); auto t3=Clock::now();
        BenchResult r2; r2.label="Intersect(100+100)"; r2.result="Size: "+std::to_string(in.size());
        r2.timeMs=std::chrono::duration<double,std::milli>(t3-t2).count(); r2.algorithm="STL set iteration";
        res.push_back(r2);
    }
    {
        auto t0=Clock::now(); long long cnt=0; for(int m=0;m<(1<<20);++m) ++cnt; auto t1=Clock::now();
        BenchResult r; r.label="Bitmask subsets (n=20)"; r.result="Generated "+std::to_string(cnt)+" subsets";
        r.timeMs=std::chrono::duration<double,std::milli>(t1-t0).count(); r.algorithm="Bit manipulation O(1) space";
        res.push_back(r);
    }
    return res;
}

// ================================================================
//  SECTION 13: UNIT TESTS
// ================================================================
struct TestResult { std::string name, detail; bool passed; TestResult():passed(false){} };

static std::vector<TestResult> runUnitTests() {
    std::vector<TestResult> all;
    auto addT = [&](const std::string& name, bool cond, const std::string& detail="") {
        TestResult t; t.name=name; t.passed=cond; t.detail=detail; all.push_back(t);
    };
    std::set<std::string> A,B;
    A.insert("a"); A.insert("b"); A.insert("c");
    B.insert("b"); B.insert("c"); B.insert("d");
    addT("Union size correct",        setUnion(A,B).size()==4,       "A union B = 4 elements");
    addT("Intersection size correct", setIntersect(A,B).size()==2,   "A intersect B = 2 elements");
    addT("Difference size correct",   setDiff(A,B).size()==1,        "A - B = 1 element");
    addT("Subset check correct",      !isSubset(A,B),                "A is not subset of B");
    addT("Power set size (2^n)",      powerSet(A).size()==8,         "PowerSet{a,b,c} = 8 subsets");
    addT("C(5,2) = 10",          comb(5,2)==10,   "");
    addT("C(10,3) = 120",        comb(10,3)==120, "");
    addT("P(5,2) = 20",          perm(5,2)==20,   "");
    addT("C(3,5) = 0 (invalid)", comb(3,5)==0,    "r > n");
    std::set<std::string> dom; dom.insert("a"); dom.insert("b"); dom.insert("c");
    Relation Rid;
    Rid.insert(std::make_pair(std::string("a"),std::string("a")));
    Rid.insert(std::make_pair(std::string("b"),std::string("b")));
    Rid.insert(std::make_pair(std::string("c"),std::string("c")));
    RelProps rp = checkRelProps(Rid, dom);
    addT("Reflexive property detected",  rp.refl,  "Identity relation");
    addT("Symmetric property detected",  rp.symm,  "Identity relation");
    addT("Transitive property detected", rp.trans, "Identity relation");
    std::map<std::string,std::string> fi, fn;
    fi["a"]="x"; fi["b"]="y"; fi["c"]="z";
    fn["a"]="x"; fn["b"]="x"; fn["c"]="z";
    std::vector<std::string> cod; cod.push_back("x"); cod.push_back("y"); cod.push_back("z");
    addT("Injective function detected",     analyseFunc(fi,cod).injective,  "distinct mappings");
    addT("Non-injective function detected", !analyseFunc(fn,cod).injective, "a,b both map to x");
    UniversityDB tdb;
    tdb.facts.insert("A");
    tdb.logicRules.push_back(std::make_pair(std::string("A"),std::string("B")));
    tdb.logicRules.push_back(std::make_pair(std::string("B"),std::string("C")));
    std::vector<InferenceTrace> tr;
    addT("Forward chaining infers B from A",       queryLogic("B",tdb,tr), "A => B");
    addT("Forward chaining infers C from A->B->C", queryLogic("C",tdb,tr), "A=>B=>C");
    addT("Cannot infer D (not in KB)",             !queryLogic("D",tdb,tr),"D not derivable");
    return all;
}

// ================================================================
//  SECTION 14: THEME SYSTEM (Light / Dark) — exact hex palette
// ================================================================
struct Theme {
    sf::Color background;     // app background
    sf::Color backgroundAlt;  // secondary background (for gradient banner end)
    sf::Color sidebar;
    sf::Color primary;        // main purple/violet
    sf::Color primary2;       // secondary violet (for gradients/glow)
    sf::Color hover;
    sf::Color text;
    sf::Color textSecondary;
    sf::Color card;
    sf::Color cardAlt;        // slightly elevated card tone
    sf::Color border;
    sf::Color glow;           // soft glow tint behind accents (low alpha)
    sf::Color success;
    sf::Color danger;
    sf::Color warning;
    sf::Color chartGreen;
    sf::Color chartOrange;
    sf::Color chartGrey;
};

// Purple / Violet glow theme — DARK mode (matches reference dashboards)
static Theme darkTheme() {
    Theme t;
    t.background     = sf::Color(0x0B,0x0A,0x14); // near-black violet
    t.backgroundAlt  = sf::Color(0x1A,0x10,0x2E); // deep violet for banner
    t.sidebar        = sf::Color(0x12,0x10,0x1F); // sidebar slightly lighter than bg
    t.primary        = sf::Color(0x8B,0x5C,0xF6); // vivid violet  #8B5CF6
    t.primary2       = sf::Color(0xC0,0x84,0xFC); // lighter violet glow
    t.hover          = sf::Color(0xA7,0x8B,0xFA); // hover lighten
    t.text           = sf::Color(0xF1,0xEC,0xFB); // near-white with violet tint
    t.textSecondary  = sf::Color(0x9C,0x94,0xB8); // muted lavender-grey
    t.card           = sf::Color(0x16,0x13,0x24); // glass card
    t.cardAlt        = sf::Color(0x1D,0x18,0x30);
    t.border         = sf::Color(0x2E,0x27,0x47);
    t.glow           = sf::Color(0x8B,0x5C,0xF6,60);
    t.success        = sf::Color(0x4A,0xDE,0x80);
    t.danger         = sf::Color(0xF8,0x71,0x71);
    t.warning        = sf::Color(0xFB,0xBF,0x24);
    t.chartGreen     = sf::Color(0x4A,0xDE,0x80);
    t.chartOrange    = sf::Color(0xFB,0xA4,0x4A);
    t.chartGrey      = sf::Color(0x4A,0x44,0x5E);
    return t;
}
// Purple / Violet glow theme — LIGHT mode (lavender-white with violet accents)
static Theme lightTheme() {
    Theme t;
    t.background     = sf::Color(0xF7,0xF4,0xFF); // crisp lavender-white
    t.backgroundAlt  = sf::Color(0xE9,0xDD,0xFF); // brighter lilac for banner
    t.sidebar        = sf::Color(0xFF,0xFF,0xFF); // white sidebar for crisp contrast
    t.primary        = sf::Color(0x8B,0x2F,0xF7); // electric violet  #8B2FF7
    t.primary2       = sf::Color(0xC8,0x4B,0xFF); // vivid magenta-violet glow
    t.hover          = sf::Color(0x73,0x18,0xE0); // deeper hover
    t.text           = sf::Color(0x1A,0x10,0x2E); // near-black violet text
    t.textSecondary  = sf::Color(0x6E,0x5E,0x8C); // muted violet-grey
    t.card           = sf::Color(0xFF,0xFF,0xFF);
    t.cardAlt        = sf::Color(0xF3,0xEC,0xFF);
    t.border         = sf::Color(0xDE,0xCC,0xFF);
    t.glow           = sf::Color(0x8B,0x2F,0xF7,40);
    t.success        = sf::Color(0x10,0xB9,0x6B); // vivid emerald
    t.danger         = sf::Color(0xF0,0x3A,0x5F); // vivid rose-red
    t.warning        = sf::Color(0xF5,0x8A,0x0E); // vivid amber-orange
    t.chartGreen     = sf::Color(0x10,0xB9,0x6B);
    t.chartOrange    = sf::Color(0xFF,0x8A,0x1E);
    t.chartGrey      = sf::Color(0xB9,0xA8,0xDE);
    return t;
}

// ================================================================
//  SECTION 15: GUI PRIMITIVES (theme-aware)
// ================================================================
struct Btn {
    sf::RectangleShape rect;
    sf::Text lbl;
    bool hov, sel;
    Btn():hov(false),sel(false){}
    void draw(sf::RenderWindow& w)const{w.draw(rect);w.draw(lbl);}
    bool hit(sf::Vector2f p)const{return rect.getGlobalBounds().contains(p);}
};
struct IBox {
    sf::RectangleShape rect;
    sf::Text txt, ph;
    std::string val;
    bool focused;
    IBox():focused(false){}
    void onEvent(const sf::Event& ev){
        if(!focused)return;
        if(ev.type==sf::Event::TextEntered){
            sf::Uint32 ch=ev.text.unicode;
            if(ch==8&&!val.empty())val.pop_back();
            else if(ch>=32&&ch<127)val+=(char)ch;
            txt.setString(val);
        }
    }
    void draw(sf::RenderWindow& w, const Theme& th){
        rect.setOutlineColor(focused?th.primary:th.border);
        rect.setOutlineThickness(focused?2.f:1.f);
        w.draw(rect);
        if(val.empty()&&!focused)w.draw(ph); else w.draw(txt);
    }
    bool hit(sf::Vector2f p)const{return rect.getGlobalBounds().contains(p);}
};

static Btn makeBtn(const std::string& s, const sf::Font& f, sf::FloatRect b, unsigned sz=14){
    Btn btn;
    btn.rect.setPosition(b.left,b.top);
    btn.rect.setSize(sf::Vector2f(b.width,b.height));
    btn.lbl=sf::Text(s,f,sz);
    sf::FloatRect tb=btn.lbl.getLocalBounds();
    btn.lbl.setPosition(b.left+16.f-tb.left, b.top+(b.height-tb.height)/2.f-tb.top-2.f);
    return btn;
}
static void updSidebarBtn(Btn& b, sf::Vector2f m, const Theme& th){
    b.hov=b.hit(m);
    if (b.sel)       b.rect.setFillColor(th.primary);
    else if (b.hov)  b.rect.setFillColor(sf::Color(th.primary.r, th.primary.g, th.primary.b, 35));
    else             b.rect.setFillColor(sf::Color::Transparent);
    b.lbl.setFillColor(b.sel ? sf::Color(255,255,255) : (b.hov ? th.primary : th.textSecondary));
}
static void updPrimaryBtn(Btn& b, sf::Vector2f m, const Theme& th){
    b.hov=b.hit(m);
    b.rect.setFillColor(b.hov ? th.hover : th.primary);
    b.lbl.setFillColor(sf::Color(255,255,255));
}
static IBox makeIBox(const std::string& ph, const sf::Font& f, sf::FloatRect b, unsigned sz=14){
    IBox ib;
    ib.rect.setPosition(b.left,b.top);
    ib.rect.setSize(sf::Vector2f(b.width,b.height));
    ib.rect.setOutlineThickness(1.f);
    ib.txt=sf::Text("",f,sz);
    ib.txt.setPosition(b.left+12,b.top+(b.height-sz)/2.f-2.f);
    ib.ph=sf::Text(ph,f,sz);
    ib.ph.setPosition(b.left+12,b.top+(b.height-sz)/2.f-2.f);
    return ib;
}
static void styleIBox(IBox& ib, const Theme& th){
    ib.rect.setFillColor(th.background);
    ib.txt.setFillColor(th.text);
    ib.ph.setFillColor(th.textSecondary);
}

static void drawCard(sf::RenderWindow& w, sf::FloatRect r, const Theme& th, float bt=1.f){
    // Soft glow behind the card edge (very subtle, gives "glass" depth)
    sf::RectangleShape glow(sf::Vector2f(r.width+10.f, r.height+10.f));
    glow.setPosition(r.left-5.f, r.top-5.f);
    glow.setFillColor(sf::Color(th.primary.r, th.primary.g, th.primary.b, 10));
    w.draw(glow);

    sf::RectangleShape rect(sf::Vector2f(r.width, r.height));
    rect.setPosition(r.left, r.top);
    rect.setFillColor(th.card);
    rect.setOutlineColor(th.border);
    rect.setOutlineThickness(bt);
    w.draw(rect);

    // Thin top accent line for "premium panel" feel
    sf::RectangleShape topAccent(sf::Vector2f(r.width, 2.f));
    topAccent.setPosition(r.left, r.top);
    topAccent.setFillColor(sf::Color(th.primary.r, th.primary.g, th.primary.b, 130));
    w.draw(topAccent);
}

// Stat tile: icon-circle + big glowing number + small label (like reference dashboard)
static void drawStatTile(sf::RenderWindow& w, const sf::Font& font, const Theme& th,
                          sf::FloatRect r, const std::string& value, const std::string& label,
                          sf::Color accent)
{
    drawCard(w, r, th, 1.f);

    // Icon circle top-left
    float cr = 18.f;
    sf::CircleShape circ(cr);
    circ.setPosition(r.left+18.f, r.top+16.f);
    circ.setFillColor(sf::Color(accent.r, accent.g, accent.b, 40));
    w.draw(circ);
    sf::CircleShape dot(5.f);
    dot.setPosition(r.left+18.f+cr-5.f, r.top+16.f+cr-5.f);
    dot.setFillColor(accent);
    w.draw(dot);

    sf::Text val(value, font, 30); val.setStyle(sf::Text::Bold);
    val.setFillColor(th.text);
    val.setPosition(r.left+18.f, r.top+62.f);
    w.draw(val);

    sf::Text lbl(label, font, 13);
    lbl.setFillColor(th.textSecondary);
    lbl.setPosition(r.left+18.f, r.top+62.f+38.f);
    w.draw(lbl);
}

// Simple donut/ring chart — used for "total" style visual on dashboard
static void drawDonut(sf::RenderWindow& w, sf::Vector2f center, float radius, float thickness,
                       const std::vector<std::pair<float,sf::Color>>& segments,
                       const Theme& th)
{
    const int SEG_STEPS = 64;
    float total = 0.f;
    for (size_t i=0;i<segments.size();++i) total += segments[i].first;
    if (total <= 0.f) total = 1.f;

    float startAngle = -90.f; // start at top
    for (size_t s=0; s<segments.size(); ++s) {
        float frac = segments[s].first / total;
        float sweep = frac * 360.f;
        int steps = std::max(2, (int)(SEG_STEPS * frac));

        sf::VertexArray ring(sf::TriangleStrip, (steps+1)*2);
        for (int i=0;i<=steps;++i) {
            float a = (startAngle + sweep * (float)i/steps) * 3.14159265f/180.f;
            float cosA = std::cos(a), sinA = std::sin(a);
            sf::Vector2f outer(center.x + (radius)*cosA, center.y + (radius)*sinA);
            sf::Vector2f inner(center.x + (radius-thickness)*cosA, center.y + (radius-thickness)*sinA);
            ring[i*2+0].position = outer; ring[i*2+0].color = segments[s].second;
            ring[i*2+1].position = inner; ring[i*2+1].color = segments[s].second;
        }
        w.draw(ring);
        startAngle += sweep;
    }
}

// ================================================================
//  SECTION 16: OUTPUT MODEL (theme-aware semantic colors)
// ================================================================
enum LineKind { LK_NORMAL, LK_DIM, LK_ACCENT, LK_OK, LK_FAIL, LK_WARN, LK_HEADING };
struct OLine { std::string text; LineKind kind; bool bold; OLine():kind(LK_NORMAL),bold(false){} OLine(const std::string&t,LineKind k,bool b):text(t),kind(k),bold(b){} };

struct AppState {
    UniversityDB db;
    int menu; float scroll;
    bool darkMode;
    std::vector<OLine> out;
    AppState():menu(0),scroll(0),darkMode(true){}
    void clear(){out.clear();scroll=0;}
    void add(const std::string&s,LineKind k=LK_NORMAL,bool b=false){out.push_back(OLine(s,k,b));}
    void hdr(const std::string&s){add("");add("  "+s,LK_HEADING,true);add(std::string(50,'-'),LK_DIM);}
    void ok(const std::string&s){add("  [SUCCESS] "+s,LK_OK,true);}
    void err(const std::string&s){add("  [ERROR] "+s,LK_FAIL,true);}
};

static sf::Color resolveColor(LineKind k, const Theme& th){
    switch(k){
        case LK_DIM:     return th.textSecondary;
        case LK_ACCENT:  return th.primary;
        case LK_OK:      return th.success;
        case LK_FAIL:    return th.danger;
        case LK_WARN:    return th.warning;
        case LK_HEADING: return th.primary;
        default:         return th.text;
    }
}

// ── Utility ───────────────────────────────────
static std::vector<std::string> csv(const std::string& s){
    std::vector<std::string> v; std::istringstream ss(s); std::string t;
    while(std::getline(ss,t,',')){
        while(!t.empty()&&t.front()==' ')t.erase(t.begin());
        while(!t.empty()&&t.back()==' ')t.pop_back();
        if(!t.empty())v.push_back(t);
    }
    return v;
}
static std::string joinV(const std::vector<std::string>& v){
    std::string r; for(size_t i=0;i<v.size();++i)r+=v[i]+(i+1<v.size()?", ":""); return r;
}
static std::string fmtS(const std::set<std::string>& s){
    std::string r="{"; int i=0;
    for(std::set<std::string>::const_iterator it=s.begin();it!=s.end();++it,++i)
        r+=*it+(i+1<(int)s.size()?", ":"");
    return r+"}";
}
static std::string fd(double d){std::ostringstream o;o<<std::fixed<<std::setprecision(4)<<d;return o.str();}

// ── Module runners ────────────────────────────
static void showDashboard(AppState& a){
    a.clear(); a.hdr("FAST UNIVERSITY DISCRETE STRUCTURES SYSTEM"); a.add("");
    a.add("  Courses     : "+std::to_string(a.db.courses.size()),LK_ACCENT,true);
    a.add("  Students    : "+std::to_string(a.db.students.size()),LK_ACCENT,true);
    a.add("  Faculty     : "+std::to_string(a.db.faculty.size()),LK_ACCENT,true);
    a.add("  Rooms       : "+std::to_string(a.db.rooms.size()),LK_ACCENT,true);
    a.add("  Logic Rules : "+std::to_string(a.db.logicRules.size()),LK_ACCENT,true);
    a.add("  Facts       : "+std::to_string(a.db.facts.size()),LK_ACCENT,true);
    a.add("");
    a.add("  Use the sidebar to add data and run modules.",LK_DIM);
    a.add("  Fill the form fields, then click Run / Submit.",LK_DIM);
}
static void showScheduling(AppState& a){
    a.clear(); a.hdr("VALID COURSE SEQUENCE - TOPOLOGICAL SORT");
    std::vector<std::string> order=topoSort(a.db);
    if(order.empty()){a.err("Cycle detected OR no courses added.");return;}
    a.add("  Recommended Order:",LK_DIM);
    for(int i=0;i<(int)order.size();++i)
        a.add("    "+std::to_string(i+1)+".  "+order[i],LK_ACCENT,true);
    a.add(""); a.ok("Total courses: "+std::to_string(order.size()));
    a.add("  Concept: Topological sort on DAG",LK_DIM);
}
static void showCombinations(AppState& a, int r){
    a.clear(); a.hdr("STUDENT GROUP COMBINATION MODULE");
    std::vector<std::string> ids=a.db.getStudentIDs(); int n=(int)ids.size();
    a.add("  Total students: "+std::to_string(n));
    a.add("  Students: "+joinV(ids)); a.add("");
    long long c=comb(n,r), p=perm(n,r);
    a.add("  C("+std::to_string(n)+","+std::to_string(r)+") = "+std::to_string(c),LK_ACCENT,true);
    a.add("  P("+std::to_string(n)+","+std::to_string(r)+") = "+std::to_string(p),LK_ACCENT,true); a.add("");
    a.add("  All Possible Groups:",LK_DIM);
    std::vector<std::vector<std::string>> grps=allCombinations(ids,r);
    int cap=(int)std::min((int)grps.size(),50);
    for(int i=0;i<cap;++i) a.add("    Group "+std::to_string(i+1)+": {"+joinV(grps[i])+"}");
    if((int)grps.size()>50) a.add("    ... and "+std::to_string(grps.size()-50)+" more",LK_DIM);
    a.add(""); a.add("  Sequential Assignment:",LK_DIM);
    std::vector<std::vector<std::string>> sg=seqGroups(ids,r);
    for(int i=0;i<(int)sg.size();++i) a.add("    Group "+std::to_string(i+1)+": {"+joinV(sg[i])+"}");
    a.add(""); a.ok("Combinations generated!");
    a.add("  Concept: C(n,r) and P(n,r) with DP memoization",LK_DIM);
}
static void showInduction(AppState& a, const std::string& sid, const std::string& code){
    a.clear(); a.hdr("PREREQUISITE VERIFICATION - MATHEMATICAL INDUCTION");
    if(!a.db.studentExists(sid)){a.err("Student '"+sid+"' not found.");return;}
    if(!a.db.courseExists(code)){a.err("Course '"+code+"' not found.");return;}
    a.add("  Target Course: "+code,LK_ACCENT,true); a.add("");
    std::vector<InductionStep> steps=verifyPrereqs(sid,code,a.db);
    if(steps.empty()){a.add("  No prerequisites required.",LK_DIM);}
    else{
        a.add("  All Prerequisites (Direct + Indirect):",LK_DIM);
        for(size_t i=0;i<steps.size();++i){
            std::string dep(steps[i].depth*2,' ');
            a.add("    "+dep+"- "+steps[i].course);
        }
        a.add(""); a.add("  Verification:",LK_DIM);
        for(size_t i=0;i<steps.size();++i){
            if(steps[i].completed) a.add("    [OK]   "+steps[i].course+" - Completed",LK_OK);
            else                   a.add("    [FAIL] "+steps[i].course+" - NOT Completed",LK_FAIL);
        }
    }
    a.add("");
    bool ok=canEnroll(sid,code,a.db);
    if(ok) a.ok("Prerequisites satisfied! Enrollment in "+code+" permitted.");
    else   a.err("Prerequisites NOT satisfied! Enrollment in "+code+" denied.");
    a.add("  Concept: Mathematical induction - base case + inductive step",LK_DIM);
}
static void showLogic(AppState& a, const std::string& qry){
    a.clear(); a.hdr("PERFORM INFERENCE");
    a.add("  Query: "+qry,LK_ACCENT,true); a.add("");
    std::vector<InferenceTrace> tr; bool res=queryLogic(qry,a.db,tr);
    a.add("  Inference trace:",LK_DIM);
    for(size_t i=0;i<tr.size();++i) a.add("    "+tr[i].step, tr[i].derived?LK_ACCENT:LK_FAIL);
    a.add("");
    if(res){a.add("  Result: TRUE",LK_OK,true); a.ok("Query can be inferred!");}
    else   {a.add("  Result: FALSE",LK_FAIL,true); a.err("Query cannot be inferred.");}
    a.add("  Concept: Forward chaining, modus ponens",LK_DIM);
}
static void showSets(AppState& a, const std::set<std::string>& A, const std::set<std::string>& B){
    a.clear(); a.hdr("SET OPERATIONS MODULE");
    a.add("  Set A: "+fmtS(A)); a.add("  Set B: "+fmtS(B)); a.add("");
    a.add("  A union B     : "+fmtS(setUnion(A,B)));
    a.add("  A intersect B : "+fmtS(setIntersect(A,B)));
    a.add("  A - B         : "+fmtS(setDiff(A,B)));
    a.add("  A subset of B : "+std::string(isSubset(A,B)?"Yes":"No")); a.add("");
    std::vector<std::set<std::string>> ps=powerSet(A);
    a.add("  Power Set of A ("+std::to_string(ps.size())+" subsets):",LK_DIM);
    int shown=(int)std::min((int)ps.size(),16);
    for(int i=0;i<shown;++i) a.add("    "+fmtS(ps[i]),LK_ACCENT);
    if((int)ps.size()>shown) a.add("    ... and "+std::to_string(ps.size()-shown)+" more",LK_DIM);
    a.add(""); a.ok("Set operations completed!");
    a.add("  Concept: Set theory - union, intersection, difference, power set",LK_DIM);
}
static void showRelations(AppState& a){
    a.clear(); a.hdr("RELATIONS MODULE");
    Relation R=buildSCRelation(a.db);
    a.add("  Building Student-Course Relation:",LK_DIM);
    for(Relation::const_iterator it=R.begin();it!=R.end();++it)
        a.add("    "+it->first+"  ->  "+it->second,LK_ACCENT);
    a.add("");
    std::set<std::string> dom;
    for(Relation::const_iterator it=R.begin();it!=R.end();++it){dom.insert(it->first);dom.insert(it->second);}
    RelProps p=checkRelProps(R,dom);
    a.add("  Relation Properties:",LK_DIM);
    a.add("    Reflexive      : "+std::string(p.refl ?"Yes":"No"), p.refl ?LK_OK:LK_FAIL);
    a.add("    Symmetric      : "+std::string(p.symm ?"Yes":"No"), p.symm ?LK_OK:LK_FAIL);
    a.add("    Transitive     : "+std::string(p.trans?"Yes":"No"), p.trans?LK_OK:LK_FAIL);
    a.add("    Antisymmetric  : "+std::string(p.anti ?"Yes":"No"), p.anti ?LK_OK:LK_FAIL);
    a.add("    Equivalence    : "+std::string(p.equiv?"Yes":"No"), p.equiv?LK_OK:LK_FAIL);
    a.add("    Partial Order  : "+std::string(p.partial?"Yes":"No"), p.partial?LK_OK:LK_FAIL);
    std::vector<std::string> dv(dom.begin(),dom.end());
    if((int)dv.size()<=10 && !dv.empty()){
        std::vector<std::vector<int>> mat=relMatrix(R,dv); a.add("");
        a.add("  Relation Matrix:",LK_DIM);
        std::string hdr="         ";
        for(size_t i=0;i<dv.size();++i){std::string col=dv[i].substr(0,6);while(col.size()<8)col+=" ";hdr+=col;}
        a.add(hdr,LK_DIM);
        for(int i=0;i<(int)dv.size();++i){
            std::string row="  "+dv[i].substr(0,6); while(row.size()<8+2)row+=" ";
            for(int j=0;j<(int)dv.size();++j) row+="   "+std::to_string(mat[i][j])+"    ";
            a.add(row);
        }
    }
    a.add(""); a.ok("Relation analysis completed!");
    a.add("  Concept: Binary relations, relation properties",LK_DIM);
}
static void showFunctions(AppState& a){
    a.clear(); a.hdr("FUNCTIONS MODULE");
    std::map<std::string,std::string> f=courseFacultyMap(a.db);
    a.add("  Course -> Faculty Mapping:",LK_DIM);
    for(std::map<std::string,std::string>::const_iterator it=f.begin();it!=f.end();++it)
        a.add("    "+it->first+"  ->  "+it->second,LK_ACCENT);
    a.add("");
    std::vector<std::string> fids=a.db.getFacultyIDs();
    FuncProps p=analyseFunc(f,fids);
    a.add("  Properties:",LK_DIM);
    a.add("    Injective (One-to-One): "+std::string(p.injective ?"Yes":"No"), p.injective ?LK_OK:LK_FAIL);
    a.add("    Surjective (Onto)     : "+std::string(p.surjective?"Yes":"No"), p.surjective?LK_OK:LK_FAIL);
    a.add("    Bijective             : "+std::string(p.bijective ?"Yes":"No"), p.bijective ?LK_OK:LK_FAIL);
    a.add(""); a.ok("Function analysis completed!");
    a.add("  Concept: Functions - injective, surjective, bijective",LK_DIM);
}
static void showProof(AppState& a, const std::set<std::string>& A, const std::set<std::string>& B){
    a.clear(); a.hdr("AUTOMATED PROOF");
    std::vector<ProofStep> steps=proveSetEquality(A,B);
    for(size_t i=0;i<steps.size();++i)
        if(!steps[i].stmt.empty()) a.add("  "+steps[i].stmt, steps[i].ok?LK_NORMAL:LK_FAIL, steps[i].bold);
    a.add(""); a.add("  Concept: Proof by mutual subset containment",LK_DIM);
}
static void showConsistency(AppState& a){
    a.clear(); a.hdr("CONSISTENCY CHECK REPORT");
    std::vector<CIssue> issues=checkConsistency(a.db);
    if(issues.empty()){a.ok("No issues found. System is consistent!");}
    else{
        a.err("Found "+std::to_string(issues.size())+" issue(s):"); a.add("");
        for(int i=0;i<(int)issues.size();++i)
            a.add("    "+std::to_string(i+1)+". ["+issues[i].cat+"] "+issues[i].desc, LK_WARN);
    }
    a.add("  Concept: Cross-module logical consistency checking",LK_DIM);
}
static void showBenchmark(AppState& a){
    a.clear(); a.hdr("ALGORITHMIC EFFICIENCY BENCHMARKING");
    std::vector<BenchResult> res=runBenchmarks();
    for(size_t i=0;i<res.size();++i){
        a.add("  "+res[i].label,LK_ACCENT,true);
        a.add("    Result    : "+res[i].result);
        a.add("    Time      : "+fd(res[i].timeMs)+" ms",LK_DIM);
        a.add("    Algorithm : "+res[i].algorithm,LK_DIM);
        a.add("");
    }
    a.ok("Benchmarking completed!");
}
static void showTests(AppState& a){
    a.clear(); a.hdr("UNIT TESTING FRAMEWORK");
    std::vector<TestResult> tests=runUnitTests();
    int passed=0,failed=0;
    for(size_t i=0;i<tests.size();++i){
        if(tests[i].passed){a.add("    [PASS] "+tests[i].name,LK_OK);++passed;}
        else{a.add("    [FAIL] "+tests[i].name+(tests[i].detail.empty()?"":" - "+tests[i].detail),LK_FAIL);++failed;}
    }
    int total=passed+failed;
    float pct=total>0?100.f*passed/total:0.f;
    a.add(""); a.add(std::string(50,'-'),LK_DIM);
    a.add("  Total: "+std::to_string(total)+"   Passed: "+std::to_string(passed)+"   Failed: "+std::to_string(failed),LK_NORMAL,true);
    std::ostringstream oss; oss<<std::fixed<<std::setprecision(2)<<pct;
    a.add("  Success Rate: "+oss.str()+"%", pct==100.f?LK_OK:LK_WARN);
    a.add(std::string(50,'-'),LK_DIM); a.add("");
    if(failed==0) a.ok("All tests passed!"); else a.err(std::to_string(failed)+" test(s) failed.");
}

// ── Live lists shown beside Add-forms, so multiple entries are visible ──
static void showCourseList(AppState& a){
    a.clear(); a.hdr("COURSES ADDED SO FAR ("+std::to_string(a.db.courses.size())+")");
    if(a.db.courses.empty()){ a.add("  No courses yet. Fill the form and click Run / Submit.",LK_DIM); return; }
    for(std::map<std::string,Course>::const_iterator it=a.db.courses.begin();it!=a.db.courses.end();++it){
        const Course& c=it->second;
        a.add("  "+c.code+"  -  "+c.name,LK_ACCENT,true);
        a.add("      Credits: "+std::to_string(c.credits)+"   Capacity: "+std::to_string(c.capacity),LK_DIM);
        if(!c.prerequisites.empty()) a.add("      Prerequisites: "+joinV(c.prerequisites),LK_DIM);
        a.add("");
    }
}
static void showStudentList(AppState& a){
    a.clear(); a.hdr("STUDENTS ADDED SO FAR ("+std::to_string(a.db.students.size())+")");
    if(a.db.students.empty()){ a.add("  No students yet. Fill the form and click Run / Submit.",LK_DIM); return; }
    for(std::map<std::string,Student>::const_iterator it=a.db.students.begin();it!=a.db.students.end();++it){
        const Student& s=it->second;
        a.add("  "+s.id+"  -  "+s.name,LK_ACCENT,true);
        a.add("      Semester: "+std::to_string(s.semester),LK_DIM);
        if(!s.completedCourses.empty()) a.add("      Completed: "+joinV(s.completedCourses),LK_DIM);
        if(!s.enrolledCourses.empty())  a.add("      Enrolled:  "+joinV(s.enrolledCourses),LK_DIM);
        a.add("");
    }
}
static void showFacultyList(AppState& a){
    a.clear(); a.hdr("FACULTY ADDED SO FAR ("+std::to_string(a.db.faculty.size())+")");
    if(a.db.faculty.empty()){ a.add("  No faculty yet. Fill the form and click Run / Submit.",LK_DIM); return; }
    for(std::map<std::string,Faculty>::const_iterator it=a.db.faculty.begin();it!=a.db.faculty.end();++it){
        const Faculty& f=it->second;
        a.add("  "+f.id+"  -  "+f.name,LK_ACCENT,true);
        a.add("      Max Load: "+std::to_string(f.maxLoad),LK_DIM);
        if(!f.assignedCourses.empty()) a.add("      Assigned: "+joinV(f.assignedCourses),LK_DIM);
        a.add("");
    }
}
static void showRoomList(AppState& a){
    a.clear(); a.hdr("ROOMS ADDED SO FAR ("+std::to_string(a.db.rooms.size())+")");
    if(a.db.rooms.empty()){ a.add("  No rooms yet. Fill the form and click Run / Submit.",LK_DIM); return; }
    for(std::map<std::string,Room>::const_iterator it=a.db.rooms.begin();it!=a.db.rooms.end();++it){
        const Room& r=it->second;
        a.add("  "+r.id+(r.isLab?"  [LAB]":"  [ROOM]"),LK_ACCENT,true);
        a.add("      Capacity: "+std::to_string(r.capacity),LK_DIM);
        if(!r.assignedCourses.empty()) a.add("      Assigned: "+joinV(r.assignedCourses),LK_DIM);
        a.add("");
    }
}
static void showLogicList(AppState& a){
    a.clear(); a.hdr("LOGIC RULES & FACTS ("+std::to_string(a.db.logicRules.size())+" rules, "+std::to_string(a.db.facts.size())+" facts)");
    if(!a.db.facts.empty()){
        a.add("  Facts:",LK_DIM);
        for(std::set<std::string>::const_iterator it=a.db.facts.begin();it!=a.db.facts.end();++it)
            a.add("    "+*it,LK_ACCENT);
        a.add("");
    }
    if(!a.db.logicRules.empty()){
        a.add("  Rules:",LK_DIM);
        for(size_t i=0;i<a.db.logicRules.size();++i)
            a.add("    IF "+a.db.logicRules[i].first+" THEN "+a.db.logicRules[i].second,LK_ACCENT);
    }
    if(a.db.facts.empty() && a.db.logicRules.empty())
        a.add("  No rules or facts yet. Fill the form and click Run / Submit.",LK_DIM);
}

// ── Seed sample data so the dashboard is not empty on first launch ──
static void seedSampleData(UniversityDB& db){
    Course c1; c1.code="CS101"; c1.name="Introduction to Programming"; c1.credits=3; c1.capacity=40;
    db.courses[c1.code]=c1;

    Course c2; c2.code="CS201"; c2.name="Data Structures"; c2.credits=3; c2.capacity=35;
    c2.prerequisites.push_back("CS101");
    db.courses[c2.code]=c2;

    Course c3; c3.code="CS301"; c3.name="Algorithms"; c3.credits=4; c3.capacity=30;
    c3.prerequisites.push_back("CS201");
    db.courses[c3.code]=c3;

    Course c4; c4.code="MATH101"; c4.name="Discrete Mathematics"; c4.credits=3; c4.capacity=50;
    db.courses[c4.code]=c4;

    Student s1; s1.id="S001"; s1.name="Ali Hassan"; s1.semester=3;
    s1.completedCourses.push_back("CS101"); s1.completedCourses.push_back("MATH101");
    s1.enrolledCourses.push_back("CS201");
    db.students[s1.id]=s1;

    Student s2; s2.id="S002"; s2.name="Fatima Khan"; s2.semester=5;
    s2.completedCourses.push_back("CS101"); s2.completedCourses.push_back("CS201"); s2.completedCourses.push_back("MATH101");
    s2.enrolledCourses.push_back("CS301");
    db.students[s2.id]=s2;

    Faculty f1; f1.id="F001"; f1.name="Dr. Muhammad Iqbal"; f1.maxLoad=3;
    f1.assignedCourses.push_back("CS101"); f1.assignedCourses.push_back("CS201");
    db.faculty[f1.id]=f1;

    Faculty f2; f2.id="F002"; f2.name="Dr. Sarah Ahmed"; f2.maxLoad=4;
    f2.assignedCourses.push_back("MATH101"); f2.assignedCourses.push_back("CS301");
    db.faculty[f2.id]=f2;

    Room r1; r1.id="R101"; r1.capacity=45; r1.isLab=false;
    r1.assignedCourses.push_back("CS101"); r1.assignedCourses.push_back("MATH101");
    db.rooms[r1.id]=r1;

    Room r2; r2.id="LAB-A"; r2.capacity=30; r2.isLab=true;
    r2.assignedCourses.push_back("CS201"); r2.assignedCourses.push_back("CS301");
    db.rooms[r2.id]=r2;

    addFact("CS101_COMPLETED", db);
    addRule("CS101_COMPLETED", "CAN_TAKE_CS201", db);
    addRule("CS201_COMPLETED", "CAN_TAKE_CS301", db);
}

// ================================================================
//  SECTION 17: MAIN  (Topbar + Sidebar + Cards, Light/Dark theme)
// ================================================================
enum MID {
    M_DASH=0,M_COURSE,M_STUDENT,M_FACULTY,M_ROOM,M_LOGIC,
    M_SCHED,M_COMB,M_INDUCT,M_LOGRUN,M_SETS,M_REL,M_FUNC,M_PROOF,M_CONS,M_BENCH,M_TEST,
    M_COUNT
};
static const char* MLBL[M_COUNT]={
    "Dashboard","Add Course","Add Student","Add Faculty","Add Room","Logic Rules",
    "Scheduling","Combinations","Induction","Logic Engine","Set Operations",
    "Relations","Functions","Proof Module","Consistency","Benchmark","Unit Tests"
};

struct Form { std::vector<IBox> fields; std::vector<std::string> lbls; Btn run; std::string status; bool statusOk; Form():statusOk(true){} };

int main() {
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    sf::RenderWindow win(desktop, "FAST University - Discrete Structures System",
                          sf::Style::Fullscreen);
    win.setFramerateLimit(60);

    int WW = (int)desktop.width;
    int WH = (int)desktop.height;
    int SW = std::max(270, WW / 6);     // sidebar width
    int HH = 72;                        // topbar height

    sf::Font font;
    bool fl=false;
    const char* fps[]={"C:\\Windows\\Fonts\\segoeui.ttf","C:\\Windows\\Fonts\\arial.ttf",nullptr};
    for(int i=0;fps[i]&&!fl;++i) fl=font.loadFromFile(fps[i]);

    sf::Font monoFont;
    bool mfl=false;
    const char* mfps[]={"C:\\Windows\\Fonts\\consola.ttf","C:\\Windows\\Fonts\\cour.ttf",nullptr};
    for(int i=0;mfps[i]&&!mfl;++i) mfl=monoFont.loadFromFile(mfps[i]);
    if(!mfl) monoFont = font;

    // ── Background textures (different image per theme) ──
    sf::Texture bgLightTex, bgDarkTex;
    bool bgLightLoaded = bgLightTex.loadFromFile("assets/bg_light.jpg");
    bool bgDarkLoaded  = bgDarkTex.loadFromFile("assets/bg_dark.jpg");
    sf::Sprite bgLightSprite, bgDarkSprite;
    if (bgLightLoaded) bgLightSprite.setTexture(bgLightTex);
    if (bgDarkLoaded)  bgDarkSprite.setTexture(bgDarkTex);

    AppState app;
    seedSampleData(app.db);
    Theme th = darkTheme();

    // ── Topbar: theme toggle button ──
    Btn themeBtn = makeBtn("Light Mode", font, sf::FloatRect((float)WW-190.f, 18.f, 150.f, 38.f), 14);

    // ── Sidebar buttons ──
    std::vector<Btn> sbtn(M_COUNT);
    float sy = (float)HH + 24.f;
    float btnH = 44.f, btnGap = 6.f;
    for(int i=0;i<M_COUNT;++i){
        if(i==1 || i==6) sy += 20.f;
        sbtn[i]=makeBtn(MLBL[i],font,sf::FloatRect(14.f,sy,(float)SW-28.f,btnH),15);
        sy += btnH + btnGap;
    }
    sbtn[0].sel=true;

    // ── Forms ──
    std::vector<Form> forms(M_COUNT);
    float FX = (float)SW + 48.f;
    float FW = (float)(WW - SW) - 96.f;
    float ROWH = 66.f;
    float labelGap = 24.f;
    float inputH = 36.f;

    auto addF=[&](int m, const std::string& lbl){
        float fy = (float)HH + 64.f + (float)forms[m].fields.size() * ROWH;
        IBox ib = makeIBox("", font, sf::FloatRect(FX, fy + labelGap, FW*0.5f, inputH), 14);
        forms[m].fields.push_back(ib);
        forms[m].lbls.push_back(lbl);
    };
    addF(M_COURSE,"Course Code"); addF(M_COURSE,"Course Name"); addF(M_COURSE,"Credits"); addF(M_COURSE,"Capacity"); addF(M_COURSE,"Prerequisites (comma-separated)");
    addF(M_STUDENT,"Student ID"); addF(M_STUDENT,"Student Name"); addF(M_STUDENT,"Semester"); addF(M_STUDENT,"Completed Courses (comma-separated)"); addF(M_STUDENT,"Enrolled Courses (comma-separated)");
    addF(M_FACULTY,"Faculty ID"); addF(M_FACULTY,"Faculty Name"); addF(M_FACULTY,"Max Load"); addF(M_FACULTY,"Assigned Courses (comma-separated)");
    addF(M_ROOM,"Room ID"); addF(M_ROOM,"Capacity"); addF(M_ROOM,"Is Lab? (y/n)"); addF(M_ROOM,"Assigned Courses (comma-separated)");
    addF(M_LOGIC,"Antecedent (IF) -- leave blank to add a Fact instead"); addF(M_LOGIC,"Consequent (THEN)"); addF(M_LOGIC,"Fact (fill only when adding a fact)");
    addF(M_COMB,"Group Size (r)");
    addF(M_INDUCT,"Student ID"); addF(M_INDUCT,"Course Code");
    addF(M_LOGRUN,"Query Term");
    addF(M_SETS,"Set A (comma-separated IDs)"); addF(M_SETS,"Set B (comma-separated IDs)");
    addF(M_PROOF,"Set A (comma-separated)");    addF(M_PROOF,"Set B (comma-separated)");

    for(int m=0;m<M_COUNT;++m){
        if(forms[m].fields.empty()) continue;
        float fy = (float)HH + 64.f + (float)forms[m].fields.size()*ROWH + 14.f;
        forms[m].run = makeBtn("Run / Submit", font, sf::FloatRect(FX, fy, 200.f, 46.f), 15);
    }

    auto hasForm=[](int m){
        return m==M_COURSE||m==M_STUDENT||m==M_FACULTY||m==M_ROOM||m==M_LOGIC||
               m==M_COMB||m==M_INDUCT||m==M_LOGRUN||m==M_SETS||m==M_PROOF;
    };

    IBox* focus=nullptr;
    showDashboard(app);

    while(win.isOpen()){
        sf::Vector2i mpi=sf::Mouse::getPosition(win);
        sf::Vector2f mp((float)mpi.x,(float)mpi.y);
        int am=app.menu;

        float formBottom = hasForm(am)
            ? (float)HH + 64.f + (float)forms[am].fields.size()*ROWH + 14.f + 46.f + 32.f
            : (float)HH + 24.f;
        float oY = formBottom;
        float oH = (float)WH - oY - 32.f;
        float oX = (float)SW + 48.f;
        float oW = (float)(WW - SW) - 96.f;
        float lineH = 23.f;
        float totL = (float)app.out.size()*lineH + 28.f;
        float maxS = std::max(0.f, totL - (oH - 56.f));

        sf::Event ev;
        while(win.pollEvent(ev)){
            if(ev.type==sf::Event::Closed) win.close();
            if(ev.type==sf::Event::KeyPressed && ev.key.code==sf::Keyboard::Escape) win.close();
            if(focus) focus->onEvent(ev);

            if(ev.type==sf::Event::MouseButtonPressed&&ev.mouseButton.button==sf::Mouse::Left){
                focus=nullptr;

                // Theme toggle
                if(themeBtn.hit(mp)){
                    app.darkMode = !app.darkMode;
                    th = app.darkMode ? darkTheme() : lightTheme();
                    themeBtn.lbl.setString(app.darkMode ? "Light Mode" : "Dark Mode");
                }

                // Sidebar nav
                for(int i=0;i<M_COUNT;++i){
                    if(sbtn[i].hit(mp)){
                        for(int j=0;j<M_COUNT;++j) sbtn[j].sel=false;
                        sbtn[i].sel=true; app.menu=i; app.scroll=0;
                        switch(i){
                        case M_DASH:    showDashboard(app);    break;
                        case M_COURSE:  showCourseList(app);   break;
                        case M_STUDENT: showStudentList(app);  break;
                        case M_FACULTY: showFacultyList(app);  break;
                        case M_ROOM:    showRoomList(app);     break;
                        case M_LOGIC:   showLogicList(app);    break;
                        case M_SCHED:   showScheduling(app);   break;
                        case M_REL:     showRelations(app);    break;
                        case M_FUNC:    showFunctions(app);    break;
                        case M_CONS:    showConsistency(app);  break;
                        case M_BENCH:   showBenchmark(app);    break;
                        case M_TEST:    showTests(app);        break;
                        default: break;
                        }
                        break;
                    }
                }
                if(hasForm(app.menu)){
                    for(size_t j=0;j<forms[app.menu].fields.size();++j){
                        forms[app.menu].fields[j].focused=false;
                        if(forms[app.menu].fields[j].hit(mp)) focus=&forms[app.menu].fields[j];
                    }
                    if(focus) focus->focused=true;
                }
                if(hasForm(app.menu)&&forms[app.menu].run.hit(mp)){
                    Form& fr=forms[app.menu];
                    auto v=[&](int i)->std::string{return i<(int)fr.fields.size()?fr.fields[i].val:"";};
                    auto clearFields=[&](){
                        for(size_t i=0;i<fr.fields.size();++i){
                            fr.fields[i].val.clear();
                            fr.fields[i].txt.setString("");
                            fr.fields[i].focused=false;
                        }
                        focus=nullptr;
                    };
                    switch(app.menu){
                    case M_COURSE:{
                        if(v(0).empty()){fr.status="Code required!";fr.statusOk=false;break;}
                        Course c; c.code=v(0);c.name=v(1);
                        c.credits=v(2).empty()?3:std::stoi(v(2));
                        c.capacity=v(3).empty()?30:std::stoi(v(3));
                        c.prerequisites=csv(v(4));
                        app.db.courses[c.code]=c;
                        fr.status="Course "+c.code+" added! ("+std::to_string(app.db.courses.size())+" total) - ready for next.";
                        fr.statusOk=true;
                        clearFields();
                        showCourseList(app);
                        break;
                    }
                    case M_STUDENT:{
                        if(v(0).empty()){fr.status="ID required!";fr.statusOk=false;break;}
                        Student s; s.id=v(0);s.name=v(1);
                        s.semester=v(2).empty()?1:std::stoi(v(2));
                        s.completedCourses=csv(v(3)); s.enrolledCourses=csv(v(4));
                        app.db.students[s.id]=s;
                        fr.status="Student "+s.id+" added! ("+std::to_string(app.db.students.size())+" total) - ready for next.";
                        fr.statusOk=true;
                        clearFields();
                        showStudentList(app);
                        break;
                    }
                    case M_FACULTY:{
                        if(v(0).empty()){fr.status="ID required!";fr.statusOk=false;break;}
                        Faculty f; f.id=v(0);f.name=v(1);
                        f.maxLoad=v(2).empty()?3:std::stoi(v(2));
                        f.assignedCourses=csv(v(3));
                        app.db.faculty[f.id]=f;
                        fr.status="Faculty "+f.id+" added! ("+std::to_string(app.db.faculty.size())+" total) - ready for next.";
                        fr.statusOk=true;
                        clearFields();
                        showFacultyList(app);
                        break;
                    }
                    case M_ROOM:{
                        if(v(0).empty()){fr.status="ID required!";fr.statusOk=false;break;}
                        Room r; r.id=v(0);
                        r.capacity=v(1).empty()?30:std::stoi(v(1));
                        r.isLab=(!v(2).empty()&&(v(2)[0]=='y'||v(2)[0]=='Y'));
                        r.assignedCourses=csv(v(3));
                        app.db.rooms[r.id]=r;
                        fr.status="Room "+r.id+" added! ("+std::to_string(app.db.rooms.size())+" total) - ready for next.";
                        fr.statusOk=true;
                        clearFields();
                        showRoomList(app);
                        break;
                    }
                    case M_LOGIC:{
                        std::string ant=v(0),con=v(1),fact=v(2);
                        if(!ant.empty()&&!con.empty()){
                            addRule(ant,con,app.db);
                            fr.status="Rule added: IF "+ant+" THEN "+con+" ("+std::to_string(app.db.logicRules.size())+" total) - ready for next.";
                            fr.statusOk=true;
                            clearFields();
                            showLogicList(app);
                        }
                        else if(!fact.empty()){
                            addFact(fact,app.db);
                            fr.status="Fact added: "+fact+" ("+std::to_string(app.db.facts.size())+" total) - ready for next.";
                            fr.statusOk=true;
                            clearFields();
                            showLogicList(app);
                        }
                        else{fr.status="Enter rule OR fact.";fr.statusOk=false;}
                        break;
                    }
                    case M_COMB: showCombinations(app,v(0).empty()?2:std::stoi(v(0))); break;
                    case M_INDUCT: showInduction(app,v(0),v(1)); break;
                    case M_LOGRUN: showLogic(app,v(0)); break;
                    case M_SETS:{
                        std::vector<std::string> av=csv(v(0)),bv=csv(v(1));
                        std::set<std::string> A(av.begin(),av.end()),B(bv.begin(),bv.end());
                        showSets(app,A,B); break;
                    }
                    case M_PROOF:{
                        std::vector<std::string> av=csv(v(0)),bv=csv(v(1));
                        std::set<std::string> A(av.begin(),av.end()),B(bv.begin(),bv.end());
                        showProof(app,A,B); break;
                    }
                    }
                }
            }
            if(ev.type==sf::Event::MouseWheelScrolled&&mp.x>(float)SW){
                app.scroll-=ev.mouseWheelScroll.delta*40.f;
                app.scroll=std::max(0.f,std::min(app.scroll,maxS));
            }
        }

        // ════════════════════════ DRAW ════════════════════════
        win.clear(th.background);

        // ── Full-screen background image (theme-specific) ──
        bool useLightBg = !app.darkMode && bgLightLoaded;
        bool useDarkBg  = app.darkMode && bgDarkLoaded;
        if (useLightBg || useDarkBg) {
            sf::Sprite& bgSpr = app.darkMode ? bgDarkSprite : bgLightSprite;
            sf::Texture& bgTexRef = app.darkMode ? bgDarkTex : bgLightTex;
            sf::Vector2u bts = bgTexRef.getSize();
            float sx = (float)WW / bts.x, sy = (float)WH / bts.y;
            bgSpr.setScale(sx, sy);
            bgSpr.setPosition(0.f, 0.f);
            sf::Color tint = app.darkMode ? sf::Color(255,255,255,235) : sf::Color(255,255,255,255);
            bgSpr.setColor(tint);
            win.draw(bgSpr);

            // Subtle overlay so text/cards stay readable over the photo background
            sf::RectangleShape overlay(sf::Vector2f((float)WW,(float)WH));
            overlay.setFillColor(app.darkMode ? sf::Color(11,10,20,150) : sf::Color(250,248,255,120));
            win.draw(overlay);
        }

        // ── Topbar ──
        sf::RectangleShape tbg(sf::Vector2f((float)WW,(float)HH));
        tbg.setFillColor(th.card);
        win.draw(tbg);
        sf::RectangleShape tLine(sf::Vector2f((float)WW, 1.f));
        tLine.setPosition(0, (float)HH-1.f);
        tLine.setFillColor(th.border);
        win.draw(tLine);

        // Logo block — simple clean geometric mark, no external image
        sf::RectangleShape logoBox(sf::Vector2f(40.f,40.f));
        logoBox.setPosition(28.f, (HH-40.f)/2.f);
        logoBox.setFillColor(th.primary);
        win.draw(logoBox);
        sf::Text logoMark("FU", font, 16); logoMark.setStyle(sf::Text::Bold);
        logoMark.setFillColor(sf::Color(255,255,255));
        sf::FloatRect lmb = logoMark.getLocalBounds();
        logoMark.setPosition(28.f+(40.f-lmb.width)/2.f-lmb.left, (HH-40.f)/2.f+(40.f-lmb.height)/2.f-lmb.top-3.f);
        win.draw(logoMark);

        sf::Text ht("FAST University",font,21); ht.setStyle(sf::Text::Bold);
        ht.setFillColor(th.text);
        ht.setPosition(82.f, 16.f);
        win.draw(ht);
        sf::Text ht2("Discrete Structures Framework",font,13);
        ht2.setFillColor(th.textSecondary);
        ht2.setPosition(82.f, 41.f);
        win.draw(ht2);

        sf::Text hs(std::string("Module: ") + MLBL[app.menu], font, 13);
        hs.setFillColor(th.textSecondary);
        sf::FloatRect hsb = hs.getLocalBounds();
        hs.setPosition((float)WW - 220.f - hsb.width - 28.f, (HH-13.f)/2.f - 1.f);
        win.draw(hs);

        updPrimaryBtn(themeBtn, mp, th);
        themeBtn.draw(win);

        // ── Sidebar ──
        sf::RectangleShape sbg(sf::Vector2f((float)SW,(float)WH-(float)HH));
        sbg.setPosition(0,(float)HH);
        sbg.setFillColor(th.sidebar);
        win.draw(sbg);
        sf::RectangleShape sLine(sf::Vector2f(1.f,(float)WH-(float)HH));
        sLine.setPosition((float)SW-1.f, (float)HH);
        sLine.setFillColor(th.border);
        win.draw(sLine);

        for(int i=0;i<M_COUNT;++i){
            updSidebarBtn(sbtn[i],mp,th);
            sbtn[i].draw(win);
        }

        // ── Form card ──
        if(hasForm(am)){
            Form& fr=forms[am];
            float cardY = (float)HH + 20.f;
            float cardH = formBottom - cardY - 12.f;
            drawCard(win, sf::FloatRect((float)SW+24.f, cardY, (float)(WW-SW)-48.f, cardH), th, 1.f);

            sf::Text cardTitle(MLBL[am], font, 19); cardTitle.setStyle(sf::Text::Bold);
            cardTitle.setFillColor(th.text);
            cardTitle.setPosition(FX, (float)HH + 36.f);
            win.draw(cardTitle);

            for(int i=0;i<(int)fr.fields.size();++i){
                float fy = (float)HH + 64.f + i*ROWH;
                sf::Text lt(fr.lbls[i],font,13);
                lt.setFillColor(th.textSecondary);
                lt.setPosition(FX, fy);
                win.draw(lt);

                fr.fields[i].rect.setPosition(FX, fy + labelGap);
                fr.fields[i].rect.setSize(sf::Vector2f(FW*0.5f, inputH));
                fr.fields[i].txt.setPosition(FX+12, fy + labelGap + (inputH-14)/2.f - 2.f);
                fr.fields[i].ph.setPosition(FX+12, fy + labelGap + (inputH-14)/2.f - 2.f);
                styleIBox(fr.fields[i], th);
                fr.fields[i].draw(win, th);
            }

            // Dynamic button label: "Add Course" the first time, "Add Another Course" after
            {
                size_t entryCount = 0;
                std::string nounSingular = "Entry";
                switch (am) {
                    case M_COURSE:  entryCount = app.db.courses.size();  nounSingular = "Course";  break;
                    case M_STUDENT: entryCount = app.db.students.size(); nounSingular = "Student"; break;
                    case M_FACULTY: entryCount = app.db.faculty.size();  nounSingular = "Faculty Member"; break;
                    case M_ROOM:    entryCount = app.db.rooms.size();    nounSingular = "Room";    break;
                    case M_LOGIC:   entryCount = app.db.logicRules.size()+app.db.facts.size(); nounSingular = "Rule / Fact"; break;
                    default: break;
                }
                std::string btnLbl = (entryCount==0) ? ("Add " + nounSingular)
                                                       : ("Add Another " + nounSingular);
                if (am == M_COMB || am == M_INDUCT || am == M_LOGRUN || am == M_SETS || am == M_PROOF)
                    btnLbl = "Run / Submit";
                fr.run.lbl.setString(btnLbl);
                sf::FloatRect rtb = fr.run.lbl.getLocalBounds();
                fr.run.lbl.setPosition(FX + 16.f - rtb.left, fr.run.rect.getPosition().y + (46.f - rtb.height)/2.f - rtb.top - 2.f);
                // widen the button a bit so longer labels fit
                float neededW = rtb.width + 32.f;
                if (neededW > fr.run.rect.getSize().x) fr.run.rect.setSize(sf::Vector2f(neededW, 46.f));
            }

            float fy2 = (float)HH + 64.f + (float)fr.fields.size()*ROWH + 14.f;
            updPrimaryBtn(fr.run,mp,th);
            fr.run.draw(win);

            if(!fr.status.empty()){
                float bx = FX + fr.run.rect.getSize().x + 16.f;
                float bw = (FX + FW) - bx;
                if (bw > 80.f) {
                    sf::RectangleShape statusBanner(sf::Vector2f(bw, 46.f));
                    statusBanner.setPosition(bx, fy2);
                    sf::Color bgc = fr.statusOk ? th.success : th.danger;
                    statusBanner.setFillColor(sf::Color(bgc.r, bgc.g, bgc.b, 30));
                    statusBanner.setOutlineColor(bgc);
                    statusBanner.setOutlineThickness(1.f);
                    win.draw(statusBanner);

                    sf::Text st(fr.status,font,13);
                    st.setFillColor(fr.statusOk ? th.success : th.danger);
                    st.setPosition(bx + 12.f, fy2 + (46.f-13.f)/2.f - 2.f);
                    win.draw(st);
                }
            }
        }

        // ── Main content: Dashboard gets visual cards; other modules get text card ──
        if (am == M_DASH) {
            // Gradient-ish banner (drawn as two stacked rects to fake a gradient)
            float bannerH = 130.f;
            sf::RectangleShape banner1(sf::Vector2f(oW+48.f, bannerH));
            banner1.setPosition(oX-24.f, oY);
            banner1.setFillColor(th.backgroundAlt);
            win.draw(banner1);
            sf::RectangleShape banner2(sf::Vector2f((oW+48.f)*0.55f, bannerH));
            banner2.setPosition(oX-24.f, oY);
            sf::Color bc = th.primary; bc.a = 70;
            banner2.setFillColor(bc);
            win.draw(banner2);
            sf::RectangleShape bannerBorder(sf::Vector2f(oW+48.f, bannerH));
            bannerBorder.setPosition(oX-24.f, oY);
            bannerBorder.setFillColor(sf::Color::Transparent);
            bannerBorder.setOutlineColor(th.border);
            bannerBorder.setOutlineThickness(1.f);
            win.draw(bannerBorder);

            sf::Text bTitle("FAST University Discrete Structures System", font, 24); bTitle.setStyle(sf::Text::Bold);
            bTitle.setFillColor(sf::Color(255,255,255));
            bTitle.setPosition(oX, oY+26.f);
            win.draw(bTitle);
            sf::Text bSub("Live snapshot of courses, students, faculty, rooms, rules and facts", font, 14);
            bSub.setFillColor(sf::Color(235,230,250));
            bSub.setPosition(oX, oY+64.f);
            win.draw(bSub);
            sf::Text bSub2("Use the sidebar to add data, then run any module to see verified results.", font, 13);
            bSub2.setFillColor(sf::Color(220,210,245));
            bSub2.setPosition(oX, oY+88.f);
            win.draw(bSub2);

            // Stat tiles row (4 tiles)
            float tilesY = oY + bannerH + 24.f;
            float tileH  = 130.f;
            float gap    = 20.f;
            float tileW  = (oW+48.f - gap*3) / 4.f;
            std::string vals[4] = {
                std::to_string(app.db.courses.size()),
                std::to_string(app.db.students.size()),
                std::to_string(app.db.faculty.size()),
                std::to_string(app.db.rooms.size())
            };
            std::string lbls[4] = { "Total Courses", "Total Students", "Total Faculty", "Total Rooms" };
            sf::Color accs[4] = { th.primary, th.chartGreen, th.chartOrange, th.primary2 };
            for (int i=0;i<4;++i) {
                float tx = oX-24.f + i*(tileW+gap);
                drawStatTile(win, font, th, sf::FloatRect(tx, tilesY, tileW, tileH), vals[i], lbls[i], accs[i]);
            }

            // Bottom row: left = quick facts card, right = donut chart card
            float rowY = tilesY + tileH + 24.f;
            float rowH = oH - (rowY - oY) - 16.f;
            float leftW = (oW+48.f) * 0.6f - 10.f;
            float rightW = (oW+48.f) - leftW - 20.f;

            // Left card: rules/facts quick info (full height now that Design Reference is removed)
            drawCard(win, sf::FloatRect(oX-24.f, rowY, leftW, rowH), th, 1.f);
            sf::Text lcTitle("System Status", font, 17); lcTitle.setStyle(sf::Text::Bold);
            lcTitle.setFillColor(th.text);
            lcTitle.setPosition(oX, rowY+18.f);
            win.draw(lcTitle);

            std::vector<std::string> info;
            info.push_back("Logic Rules Defined : " + std::to_string(app.db.logicRules.size()));
            info.push_back("Facts Asserted       : " + std::to_string(app.db.facts.size()));
            info.push_back("");
            info.push_back("Next steps:");
            info.push_back("  1. Add Course / Student / Faculty / Room from sidebar");
            info.push_back("  2. Add Logic Rules to enable inference queries");
            info.push_back("  3. Run Scheduling, Induction, Sets, Relations, etc.");
            info.push_back("  4. Check Consistency to validate the whole system");
            info.push_back("");
            info.push_back("Tip: every Add screen lets you keep adding");
            info.push_back("entries one after another without leaving the form.");
            float iy = rowY + 56.f;
            for (size_t i=0;i<info.size();++i) {
                sf::Text it(info[i], monoFont, 14);
                it.setFillColor(i<2 ? th.primary2 : th.textSecondary);
                it.setPosition(oX+4.f, iy);
                win.draw(it);
                iy += 22.f;
            }

            // Right card: donut chart showing data distribution
            float rcX = oX-24.f + leftW + 20.f;
            drawCard(win, sf::FloatRect(rcX, rowY, rightW, rowH), th, 1.f);
            sf::Text rcTitle("Data Distribution", font, 17); rcTitle.setStyle(sf::Text::Bold);
            rcTitle.setFillColor(th.text);
            rcTitle.setPosition(rcX+24.f, rowY+18.f);
            win.draw(rcTitle);

            float totalEntities = (float)(app.db.courses.size()+app.db.students.size()+app.db.faculty.size()+app.db.rooms.size());
            std::vector<std::pair<float,sf::Color>> segs;
            segs.push_back(std::make_pair((float)app.db.courses.size()+0.0001f, th.primary));
            segs.push_back(std::make_pair((float)app.db.students.size()+0.0001f, th.chartGreen));
            segs.push_back(std::make_pair((float)app.db.faculty.size()+0.0001f, th.chartOrange));
            segs.push_back(std::make_pair((float)app.db.rooms.size()+0.0001f, th.chartGrey));

            sf::Vector2f dCenter(rcX + rightW/2.f, rowY + rowH/2.f - 10.f);
            float dRadius = std::min(rightW, rowH) * 0.28f;
            drawDonut(win, dCenter, dRadius, dRadius*0.32f, segs, th);

            std::ostringstream pctOss;
            std::string centerLbl = totalEntities < 1.f ? "0" : std::to_string((int)totalEntities);
            sf::Text dCenterTxt(centerLbl, font, 26); dCenterTxt.setStyle(sf::Text::Bold);
            dCenterTxt.setFillColor(th.text);
            sf::FloatRect dctb = dCenterTxt.getLocalBounds();
            dCenterTxt.setPosition(dCenter.x-dctb.width/2.f-dctb.left, dCenter.y-dctb.height/2.f-dctb.top-10.f);
            win.draw(dCenterTxt);
            sf::Text dCenterSub("Total", font, 12);
            dCenterSub.setFillColor(th.textSecondary);
            sf::FloatRect dcsb = dCenterSub.getLocalBounds();
            dCenterSub.setPosition(dCenter.x-dcsb.width/2.f-dcsb.left, dCenter.y+16.f);
            win.draw(dCenterSub);

            // Legend
            float legY = rowY + rowH - 96.f;
            std::string legLbls[4] = {"Courses","Students","Faculty","Rooms"};
            for (int i=0;i<4;++i) {
                sf::RectangleShape sw(sf::Vector2f(10.f,10.f));
                sw.setPosition(rcX+24.f, legY + i*20.f);
                sw.setFillColor(segs[i].second);
                win.draw(sw);
                sf::Text lt(legLbls[i] + "  (" + std::to_string((int)(segs[i].first<1.f?0:segs[i].first)) + ")", monoFont, 12);
                lt.setFillColor(th.textSecondary);
                lt.setPosition(rcX+40.f, legY + i*20.f - 2.f);
                win.draw(lt);
            }

        } else {
            // ── Output card (text-based, for the 16 computational modules) ──
            drawCard(win, sf::FloatRect(oX-24.f, oY, oW+48.f, oH), th, 1.f);

            sf::Text outTitle(MLBL[am], font, 18); outTitle.setStyle(sf::Text::Bold);
            outTitle.setFillColor(th.text);
            outTitle.setPosition(oX, oY + 18.f);
            win.draw(outTitle);
            sf::RectangleShape headerLine(sf::Vector2f(oW+48.f, 1.f));
            headerLine.setPosition(oX-24.f, oY + 56.f);
            headerLine.setFillColor(th.border);
            win.draw(headerLine);

            sf::View sv=win.getView();
            sf::Vector2u ws=win.getSize();
            float clipY = oY + 64.f, clipH = oH - 80.f;
            sf::View cv;
            cv.setViewport(sf::FloatRect((oX-8.f)/ws.x, clipY/ws.y, (oW+16.f)/ws.x, clipH/ws.y));
            cv.setSize(oW+16.f, clipH);
            cv.setCenter((oW+16.f)/2.f, clipH/2.f + app.scroll);
            win.setView(cv);

            float ly=8.f;
            for(size_t i=0;i<app.out.size();++i){
                sf::Text t(app.out[i].text, monoFont, 15);
                t.setFillColor(resolveColor(app.out[i].kind, th));
                if(app.out[i].bold) t.setStyle(sf::Text::Bold);
                t.setPosition(12.f, ly);
                win.draw(t);
                ly += lineH;
            }
            win.setView(sv);

            if(totL>clipH){
                float bH = clipH * (clipH/totL);
                float bY = clipY + (app.scroll/maxS) * (clipH-bH);
                sf::RectangleShape bar(sf::Vector2f(6,bH));
                bar.setPosition(oX+oW+14.f, bY);
                bar.setFillColor(th.primary);
                win.draw(bar);
            }
        }

        win.display();
    }
    return 0;
}
