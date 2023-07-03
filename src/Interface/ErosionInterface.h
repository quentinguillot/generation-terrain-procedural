#ifndef EROSIONINTERFACE_H
#define EROSIONINTERFACE_H


class ErosionInterface;
#include <QWidget>
#include "Interface/ActionInterface.h"
#include "TerrainGen/VoxelGrid.h"
#include "TerrainModification/UnderwaterErosion.h"
#include "Interface/Viewer.h"

#include "Interface/FancySlider.h"

class ErosionInterface : public ActionInterface
{
    Q_OBJECT
public:
    ErosionInterface(QWidget *parent = nullptr);

    void affectTerrains(std::shared_ptr<Heightmap> heightmap, std::shared_ptr<VoxelGrid> voxelGrid, std::shared_ptr<LayerBasedGrid> layerGrid, std::shared_ptr<ImplicitNaryOperator> implicitPatch = nullptr);

    void display(Vector3 camPos = Vector3(false));

    void replay(nlohmann::json action);

    QLayout* createGUI();

    enum PARTICLE_INITIAL_LOCATION {SKY, RIVER, RANDOM, RIVER2, UNDERWATER, CENTER_TOP, FROM_X, EVERYWHERE, JUST_ABOVE_VOXELS};

public Q_SLOTS:
    void show();
    void hide();

    void throwFromCam();
    void throwFromSky();
    void throwFromSide();
    void throwFrom(PARTICLE_INITIAL_LOCATION location);

    void testManyManyErosionParameters();

    virtual void afterTerrainUpdated();

    void browseWaterFlowFromFile();
    void browseAirFlowFromFile();
    void browseDensityFieldFromFile();

    void computePredefinedRocksLocations();

public:
//    std::shared_ptr<VoxelGrid> voxelGrid;
    std::shared_ptr<UnderwaterErosion> erosion;
    Viewer* viewer;

protected:
    std::function<Vector3(Vector3)> computeFlowfieldFunction();
    Mesh rocksPathSuccess;
    Mesh rocksPathFailure;

    float erosionSize = 8.f;
    float erosionStrength = .35f;
    int erosionQtt = 450;
    float rockRandomness = .1f;

    float gravity = .981f;
    float bouncingCoefficient = 0.15f; // 1.f;
    float bounciness = 1.f;
    float minSpeed = .1f;
    float maxSpeed = 1000.f;
    float maxCapacityFactor = 1.f;
    float erosionFactor = 1.f;
    float depositFactor = 1.f;
    float matterDensity = 500.f;
    float materialImpact = 0.f;

    float airFlowfieldRotation = 0.f;
    float waterFlowfieldRotation = 90.f;
    float airForce = 0.f;
    float waterForce = 1.f;

    float dt = 1.f;

    float shearingStressConstantK = 1.f;
    float shearingRatePower = .5f;
    float erosionPowerValue = 1.f;
    float criticalShearStress = .8f;

    int numberOfIterations = 1;

    float initialCapacity = .0f;

    UnderwaterErosion::EROSION_APPLIED applyOn = UnderwaterErosion::EROSION_APPLIED::HEIGHTMAP;

    bool displayTrajectories = false;

    std::string waterFlowImagePath = "";
    std::string airFlowImagePath = "";
    std::string densityFieldImagePath = "";

    UnderwaterErosion::FLOWFIELD_TYPE flowfieldUsed = UnderwaterErosion::FLOWFIELD_TYPE::BASIC;
    UnderwaterErosion::DENSITY_TYPE densityUsed = UnderwaterErosion::DENSITY_TYPE::NATIVE;

    std::map<PARTICLE_INITIAL_LOCATION, std::vector<std::vector<std::pair<Vector3, Vector3>>>> initialPositionsAndDirections;

    QHBoxLayout* erosionLayout = nullptr;

    UnderwaterErosion erosionProcess;
    bool currentlyModifyingTerrain = false;
};

#endif // EROSIONINTERFACE_H
