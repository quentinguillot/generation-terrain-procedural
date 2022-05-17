#ifndef TERRAINGENERATIONINTERFACE_H
#define TERRAINGENERATIONINTERFACE_H

class TerrainGenerationInterface;
#include "Interface/ActionInterface.h"
#include "TerrainGen/VoxelGrid.h"
#include "TerrainGen/Grid.h"
#include "Interface/Viewer.h"

class TerrainGenerationInterface : public ActionInterface
{
    Q_OBJECT
public:
    TerrainGenerationInterface(QWidget *parent = nullptr);

    void display(MapMode mapMode, SmoothingAlgorithm smoothingAlgorithm, bool displayParticles);

    void createTerrainFromNoise(int nx, int ny, int nz, float blockSize, float noise_shifting = 0.0);
    void createTerrainFromFile(std::string filename, std::vector<std::shared_ptr<ActionInterface>> actionInterfaces = std::vector<std::shared_ptr<ActionInterface>>());
    void saveTerrain(std::string filename);

    void replay(nlohmann::json action);

    QLayout* createGUI();

Q_SIGNALS:
    void updated();

public Q_SLOTS:
    void show();
    void hide();

    void prepareShader();



public:
    std::shared_ptr<VoxelGrid> voxelGrid;
    std::shared_ptr<Grid> heightmapGrid;

protected:
    void regenerateRocksAndParticles();

    Mesh marchingCubeMesh;
    GLuint dataFieldTex;
    GLuint edgeTableTex, triTableTex;

    // TODO : Transform this into a "particle" system
    std::vector<Mesh> possibleRocks;
    std::vector<std::tuple<int, Vector3, float>> rocksIndicesAndPositionAndSize;
    int numberOfRocksDisplayed = 5000;

    Mesh particlesMesh;
    std::vector<Vector3> randomParticlesPositions;

    std::chrono::system_clock::time_point startingTime;
};

#endif // TERRAINGENERATIONINTERFACE_H
