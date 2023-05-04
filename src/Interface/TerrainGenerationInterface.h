#ifndef TERRAINGENERATIONINTERFACE_H
#define TERRAINGENERATIONINTERFACE_H

class TerrainGenerationInterface;
#include "Interface/ActionInterface.h"
#include "TerrainGen/VoxelGrid.h"
#include "TerrainGen/Heightmap.h"
#include "Interface/Viewer.h"

class TerrainGenerationInterface : public ActionInterface
{
    Q_OBJECT
public:
    TerrainGenerationInterface(QWidget *parent = nullptr);

    void display();
    void displayWaterLevel();

    void createTerrainFromNoise(int nx, int ny, int nz/*, float blockSize*/, float noise_shifting = 0.0);
    void createTerrainFromFile(std::string filename, std::map<std::string, std::shared_ptr<ActionInterface>> actionInterfaces = std::map<std::string, std::shared_ptr<ActionInterface>>());
    void createTerrainFromBiomes(nlohmann::json json_content);
    void createTerrainFromImplicitPatches(nlohmann::json json_content);
    void saveTerrain(std::string filename);

    void reloadShaders();

    void replay(nlohmann::json action);

    QLayout* createGUI();

public Q_SLOTS:
    void show();
    void hide();

    void prepareShader();

    void setWaterLevel(float newLevel);

    void updateDisplayedView(Vector3 newVoxelGridOffset, float newVoxelGridScaling);
    void reloadTerrain(std::map<std::string, std::shared_ptr<ActionInterface>> actionInterfaces = std::map<std::string, std::shared_ptr<ActionInterface>>());

    void afterTerrainUpdated();


public:
    float minIsoLevel = -1000.0;
    float maxIsoLevel =  1000.0;

    bool biomeGenerationNeeded = false;
    nlohmann::json biomeGenerationModelData;

    std::map<std::string, int> colorTexturesIndex;
    std::map<std::string, int> normalTexturesIndex;
    std::map<std::string, int> displacementTexturesIndex;

    void setVisu(MapMode _mapMode, SmoothingAlgorithm _smoothingAlgorithm, bool _displayParticles);
    MapMode mapMode = MapMode::VOXEL_MODE;
    SmoothingAlgorithm smoothingAlgorithm = SmoothingAlgorithm::MARCHING_CUBES;
    bool displayParticles = false;

protected:
    Mesh marchingCubeMesh;
    GLuint dataFieldTex;
    Mesh heightmapMesh;
    GLuint heightmapFieldTex, biomeFieldTex;
    GLuint edgeTableTex, triTableTex;
    GLuint biomesAndDensitiesTex;

    Mesh layersMesh;
    Mesh implicitMesh;

    GLuint allBiomesColorTextures, allBiomesNormalTextures, allBiomesDisplacementTextures;

    Vector3 voxelGridOffset = Vector3(0, 0, 0);
    float voxelGridScaling = 1.f;

    Mesh particlesMesh;
    std::vector<Vector3> randomParticlesPositions;

    std::string lastLoadedMap = "";


    std::chrono::system_clock::time_point startingTime;
    size_t voxelsPreviousHistoryIndex = -1;
    size_t layersPreviousHistoryIndex = -1;

public:
    Mesh waterLevelMesh;
    float waterLevel = 0.0f;
};

#endif // TERRAINGENERATIONINTERFACE_H
