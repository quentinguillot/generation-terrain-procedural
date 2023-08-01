#ifndef ENVOBJECT_H
#define ENVOBJECT_H

#include "Utils/ShapeCurve.h"
#include "Utils/json.h"
#include "DataStructure/Matrix3.h"
#include "DataStructure/Vector3.h"

#include "TerrainGen/ImplicitPatch.h"

class EnvPoint;
class EnvCurve;
class EnvArea;


class EnvObject
{
public:
    EnvObject();

    static void readFile(std::string filename);

    static EnvObject* fromJSON(nlohmann::json content);


    std::string name;
    std::function<float(Vector3)> fittingFunction;
    Vector3 flowEffect;
    float sandEffect;

    TerrainTypes material;
    ImplicitPatch::PredefinedShapes implicitShape;

    virtual float getSqrDistance(const Vector3& position, std::string complement = "") = 0;
    virtual Vector3 getVector(const Vector3& position, std::string complement = "") = 0;
    virtual EnvObject* clone() = 0;


    virtual void applySandDeposit() = 0;
    virtual void applyFlowModifcation() = 0;


    static std::function<float(Vector3)> parseFittingFunction(std::string formula);


    static Matrix3<Vector3> flowfield;
    static Matrix3<Vector3> terrainNormals;
    static Matrix3<float> sandDeposit;

    static std::map<std::string, EnvObject*> availableObjects;
    static std::vector<EnvObject*> instantiatedObjects;

    static std::pair<std::string, std::string> extractNameAndComplement(std::string variable);
    static std::pair<float, EnvObject*> getSqrDistanceTo(std::string objectName, const Vector3& position);
    static std::pair<Vector3, EnvObject*> getVectorOf(std::string objectName, const Vector3& position);

    static EnvObject* instantiate(std::string objectName);

    static void applyEffects();
};

class EnvPoint : public EnvObject {
public:
    EnvPoint();

    static EnvObject* fromJSON(nlohmann::json content);

    Vector3 position;
    float radius;

    virtual float getSqrDistance(const Vector3& position, std::string complement = "");
    virtual Vector3 getVector(const Vector3& position, std::string complement = "");
    virtual EnvPoint* clone();
    virtual void applySandDeposit();
    virtual void applyFlowModifcation();
};

class EnvCurve : public EnvObject {
public:
    EnvCurve();

    static EnvObject* fromJSON(nlohmann::json content);

    BSpline curve;
    float width;
    float length;

    virtual float getSqrDistance(const Vector3& position, std::string complement = "");
    virtual Vector3 getVector(const Vector3& position, std::string complement = "");
    virtual EnvCurve* clone();
    virtual void applySandDeposit();
    virtual void applyFlowModifcation();
};

class EnvArea : public EnvObject {
public:
    EnvArea();

    static EnvObject* fromJSON(nlohmann::json content);

    ShapeCurve area;
    float width;

    virtual float getSqrDistance(const Vector3& position, std::string complement = "");
    virtual Vector3 getVector(const Vector3& position, std::string complement = "");
    virtual EnvArea* clone();
    virtual void applySandDeposit();
    virtual void applyFlowModifcation();
};

#endif // ENVOBJECT_H