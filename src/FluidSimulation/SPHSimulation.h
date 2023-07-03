#ifndef SPHSIMULATION_H
#define SPHSIMULATION_H

#include <vector>
#include "DataStructure/Matrix3.h"
#include "DataStructure/Vector3.h"
#include "FluidSimulation/FluidSimulation.h"

namespace SPH
{

class Particle;
class SPHSimulation;
class KDNode;
class KDTree;


class Particle {
public:
    Vector3 position;
    Vector3 velocity;
    Vector3 force;
    float density;
    float pressure;
    float mass;
    float smoothingRadius;
    float gasConstant;
    float restDensity;
    float viscosity;
    bool isGhost;

    int index;
};

class SPHSimulation : public FluidSimulation
{
public:
    SPHSimulation();
    virtual ~SPHSimulation();

    std::vector<Particle> particles;
    Vector3 dimensions;
    KDTree* tree;

    int nbParticles;
    float dt;
    float damping;
    float t;
    float computeTime;

    std::vector<std::vector<size_t>> precomputedNeighbors;

    void computeNeighbors() ;
    void initialize(std::vector<std::vector<Vector3>> meshBoundaries = {});
    void computeDensityAndPressure();
    void computeForces();
    void integrate();
    void relaxDensity();
    void handleCollisions();
    void step();
    Matrix3<Vector3> getVelocities(int newSizeX, int newSizeY, int newSizeZ);
    void addVelocity(int x, int y, int z, Vector3 amount);

    std::vector<size_t> getNeighbors(Vector3& position, float distance);

};


class KDNode {
public:
    size_t particleIndex;
    KDNode* left;
    KDNode* right;
    int axis;

    KDNode(size_t pIndex, int a);
};

class KDTree {
public:
    KDNode* root;

    KDTree(std::vector<Particle>& particles);

    KDNode* build(std::vector<Particle> particles, int depth);

    void findNeighbors(std::vector<Particle>& particles, KDNode* node, Vector3& position, float maxDistance, std::vector<size_t>& neighbors);
};

}

#endif // SPHSIMULATION_H