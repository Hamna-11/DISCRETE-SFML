#include "UniversityDB.h"
#include <set>
#include <vector>
#include <string>
#include <map>

typedef std::set<std::pair<std::string, std::string>> Relation;

struct RelationProperties {
    bool reflexive;
    bool symmetric;
    bool transitive;
    bool antisymmetric;
    bool equivalence;
    bool partialOrder;

    RelationProperties() : reflexive(false), symmetric(false), transitive(false),
        antisymmetric(false), equivalence(false), partialOrder(false) {
    }
};

class RelationsModule {
public:
    static Relation buildStudentCourseRelation(const UniversityDB& db);
    static Relation buildFacultyCourseRelation(const UniversityDB& db);
    static Relation buildPrerequisiteRelation(const UniversityDB& db);
    static RelationProperties checkProperties(const Relation& R, const std::set<std::string>& domain);
    static Relation transitiveClosure(const Relation& R);
    static Relation compose(const Relation& R1, const Relation& R2);
    static std::vector<std::vector<int>> toMatrix(const Relation& R, const std::vector<std::string>& domain);
};