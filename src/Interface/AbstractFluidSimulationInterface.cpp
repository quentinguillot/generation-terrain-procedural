#include "AbstractFluidSimulationInterface.h"

AbstractFluidSimulationInterface::AbstractFluidSimulationInterface(std::string actionTypeName, QWidget *parent) : ActionInterface(actionTypeName, parent)
{
}

void AbstractFluidSimulationInterface::affectTerrains(std::shared_ptr<Heightmap> heightmap, std::shared_ptr<VoxelGrid> voxelGrid, std::shared_ptr<LayerBasedGrid> layerGrid, std::shared_ptr<ImplicitNaryOperator> implicitPatch)
{
    ActionInterface::affectTerrains(heightmap, voxelGrid, layerGrid, implicitPatch);

    const char* vParticleShader = "src/Shaders/particle.vert";
    const char* gParticleShader = "src/Shaders/particle.geom";
    const char* fParticleShader = "src/Shaders/particle.frag";
    const char* vNoShader = "src/Shaders/no_shader.vert";
    const char* fNoShader = "src/Shaders/no_shader.frag";

    this->particlesMesh = Mesh(std::make_shared<Shader>(vParticleShader, fParticleShader, gParticleShader), true, GL_POINTS);
    this->particlesMesh.useIndices = false;
    this->vectorsMesh = Mesh(std::make_shared<Shader>(vNoShader, fNoShader), true, GL_LINES);
    this->vectorsMesh.useIndices = false;
    this->boundariesMesh = Mesh(std::make_shared<Shader>(vNoShader, fNoShader));
    this->boundariesMesh.useIndices = false;

    for (int x = 0; x < _simulation->dimensions.x; x++) {
        for (int y = 0; y < _simulation->dimensions.y; y++) {
            for (int z = 0; z < _simulation->dimensions.z; z++) {
                if (x == 0)
                    this->_simulation->setVelocity(0, y, z, Vector3(.1f, 0.f, 0.f));
                else
                    this->_simulation->setVelocity(0, y, z, Vector3());
            }
        }
    }

    this->updateBoundariesMesh();
    this->updateSimulationMeshes();
}

void AbstractFluidSimulationInterface::display(Vector3 camPos)
{
    if (!this->isVisible())
        return;
    if (computeAtEachFrame) {
        this->computeSimulation(nbComputationsPerFrame);
        updateSimulationMeshes();
    }

    if (displayBoundaries) {
        boundariesMesh.shader->setVector("color", std::vector<float>{0.f, 1.f, 0.f, .4f});
        boundariesMesh.display();
        boundariesMesh.shader->setVector("color", std::vector<float>{0.f, 0.f, 0.f, .4f});
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        boundariesMesh.display();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    if (displayParticles) {
        particlesMesh.shader->setVector("color", std::vector<float>{0.f, .5f, 1.f, .4f});
        particlesMesh.reorderVertices(camPos);
        particlesMesh.display(/*GL_POINTS*/);
    }
    if (displayVectors) {
        vectorsMesh.shader->setVector("color", std::vector<float>{0.f, .5f, 1.f, .8f});
//        vectorsMesh.reorderLines(camPos);
        vectorsMesh.display(/*GL_LINES,*/ 3);
    }
}

void AbstractFluidSimulationInterface::replay(nlohmann::json action)
{
    // ActionInterface::replay(action);
}

QLayout *AbstractFluidSimulationInterface::createGUI()
{
    QVBoxLayout* layout = new QVBoxLayout();

    QCheckBox* displayBoundariesButton = new QCheckBox("Display boundaries");
    QCheckBox* displayParticlesButton = new QCheckBox("Display particles");
    QCheckBox* displayVectorsButton = new QCheckBox("Display vectors");
    QCheckBox* autoComputeButton = new QCheckBox("Compute at each frame");
    QPushButton* computeButton = new QPushButton("Compute");

    layout->addWidget(displayBoundariesButton);
    layout->addWidget(displayParticlesButton);
    layout->addWidget(displayVectorsButton);
    layout->addWidget(autoComputeButton);
    layout->addWidget(computeButton);

    displayBoundariesButton->setChecked(this->displayBoundaries);
    displayParticlesButton->setChecked(this->displayParticles);
    displayVectorsButton->setChecked(this->displayVectors);
    autoComputeButton->setChecked(this->computeAtEachFrame);

    QObject::connect(displayBoundariesButton, &QCheckBox::toggled, this, [=](bool cheecked) { this->displayBoundaries = cheecked; });
    QObject::connect(displayParticlesButton, &QCheckBox::toggled, this, [=](bool cheecked) { this->displayParticles = cheecked; });
    QObject::connect(displayVectorsButton, &QCheckBox::toggled, this, [=](bool cheecked) { this->displayVectors = cheecked; });
    QObject::connect(autoComputeButton, &QCheckBox::toggled, this, [=](bool cheecked) { this->computeAtEachFrame = cheecked; });
    QObject::connect(computeButton, &QPushButton::pressed, this, [=]() { computeSimulation(this->nbComputationsPerFrame); });

    return layout;
}

void AbstractFluidSimulationInterface::updateVectorsMesh()
{
    Matrix3<Vector3> velocities = _simulation->getVelocities(20, 20, 20);
    Mesh::createVectorField(velocities, this->voxelGrid->getDimensions(), &vectorsMesh, 2.f);
}

void AbstractFluidSimulationInterface::updateSimulationMeshes()
{
    std::cout << timeIt([=]() {
        this->updateVectorsMesh();
        this->updateParticlesMesh();
//        this->updateBoundariesMesh();
    }) << "ms render" << std::endl;
}

void AbstractFluidSimulationInterface::show()
{
    ActionInterface::show();
}

void AbstractFluidSimulationInterface::hide()
{
    ActionInterface::hide();
}

void AbstractFluidSimulationInterface::afterTerrainUpdated()
{
    for (int x = 0; x < _simulation->dimensions.x; x++) {
        for (int y = 0; y < _simulation->dimensions.y; y++) {
            for (int z = 0; z < _simulation->dimensions.z; z++) {
                if (x == 0)
                    this->_simulation->setVelocity(0, y, z, Vector3(.1f, 0, -.1f));
                else
                    this->_simulation->setVelocity(0, y, z, Vector3());
            }
        }
    }

    this->updateBoundariesMesh();
    this->updateSimulationMeshes();
}

void AbstractFluidSimulationInterface::updateParticlesMesh()
{

}

void AbstractFluidSimulationInterface::updateBoundariesMesh()
{
    if (voxelGrid == nullptr) return;
    Vector3 finalDimensions = voxelGrid->getDimensions();

    Matrix3<float> bigValues = voxelGrid->getVoxelValues();
    Matrix3<float> values = bigValues.resize(20, 20, 10);
    Mesh m;
    auto triangles = m.applyMarchingCubes(values).getTriangles();
    for (auto& tri : triangles) {
        for (auto& p : tri) {
            p /= values.getDimensions();
            p *= _simulation->dimensions;
        }
    }

//    _simulation->setObstacles(triangles);
    _simulation->setObstacles(values.binarize());

    std::vector<Vector3> allVertices;
    allVertices.reserve(triangles.size() * 3);
    for (size_t iTriangle = 0; iTriangle < triangles.size(); iTriangle++) {
        const auto& triangle = triangles[iTriangle];
        for (const auto& vertex : triangle) {
            allVertices.push_back(vertex * (finalDimensions / _simulation->dimensions));
        }
    }
    boundariesMesh.fromArray(allVertices);
}

void AbstractFluidSimulationInterface::computeSimulation(int nbSteps)
{
    for (int i = 0; i < nbSteps; i++)
        _simulation->step();
}
