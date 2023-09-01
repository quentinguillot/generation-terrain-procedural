#include "DataStructure/BVH.h"
#include "Utils/Globals.h"
#include "UnderwaterErosion.h"
#include "TerrainModification/RockErosion.h"
#include "Utils/BSpline.h"
#include "Karst/KarstHole.h"
#include "Utils/Utils.h"
#include "Graph/Matrix3Graph.h"
#include "EnvObject/EnvObject.h"

UnderwaterErosion::UnderwaterErosion()
{

}
UnderwaterErosion::UnderwaterErosion(VoxelGrid *grid, int maxRockSize, float maxRockStrength, int rockAmount)
    : voxelGrid(grid), maxRockSize(maxRockSize), rockAmount(rockAmount), maxRockStrength(maxRockStrength)
{

}
/*
void retroChangeFlowfield(std::vector<Vector3>& coords, std::vector<Vector3>& dirs, std::shared_ptr<VoxelGrid> grid)
{
    if (coords.size() == 0)
        return;
    Vector3& impactZone = coords[coords.size() - 1];
    for (size_t i = 0; i < coords.size(); i++)
    {
        Vector3& coord = coords[i];
        Vector3& dir = dirs[i];
        float alpha_effect = 0.5 * ((coord - impactZone).norm2() < 20.0 ? -2.0 : 1.0); // Inverse and double if it's a choc (last coord)
        grid->affectFlowfieldAround(coord, dir * alpha_effect, 3);
//        grid->affectFlowfieldAround(coord, alpha_effect, 10);
    }
}
*/
Vector3 getGravityForce(Vector3& position, float gravity, float particleMass, float particleVolume, float particleDensity, float environmentDensity, const Vector3& flowfieldValues, Vector3& gravityDirection) {
    // Gravity + Buoyancy
    float gravityForce = gravity * particleMass; // = gravityfieldValues.at(pos + dir);
    float boyancyForce = -environmentDensity * particleVolume; // B = - rho_fluid * V * g
    float gravityCoefficient = gravityForce + boyancyForce;
//    float gravityCoefficient = gravity * particleVolume * (particleDensity - environmentDensity); // Same as F + B
//                std::cout << environmentDensity << " -> " << particleVolume << " * (" << environmentDensity << " - " << matterDensity << ") => " << gravityCoefficient << "\n";
    // float gravityCoefficient = std::max(1.f - (environmentDensity / matterDensity), -1.f); // Keep it between -1 and 1
    Vector3 acceleration = /*flowfieldValues.at(position) +*/ gravityDirection * (gravityForce * gravityCoefficient);
    return acceleration;
}

Vector3 getSettlingForce(float particleDensity, float environmentDensity, float gravity, float capacity, float maxCapacity, Vector3& gravityDirection) {
    // Settling
    float constant = 2.f/9.f;
    float r = 1.f;
    float capacityHinderingExponent = 5.f;
//                acceleration += std::clamp(constant * r * r * (matterDensity - environmentDensity) * gravity * (std::pow(capacity/maxCapacity, capacityHinderingExponent)), -1.f, 1.f);
    Vector3 acceleration = gravityDirection * (constant * r * r * (particleDensity - environmentDensity) * gravity * (1.f - std::pow(capacity/maxCapacity, capacityHinderingExponent)));
    return acceleration;
}
Vector3 getSettlingVelocity(float particleDensity, float particleSize, float environmentDensity, float gravity, float capacity, float maxCapacity, Vector3& gravityDirection) {
    // Settling
    float constant = 2.f/9.f;
    float r = particleSize;
    float capacityHinderingExponent = 5.f;
//                acceleration += std::clamp(constant * r * r * (matterDensity - environmentDensity) * gravity * (std::pow(capacity/maxCapacity, capacityHinderingExponent)), -1.f, 1.f);
    Vector3 acceleration = gravityDirection * (constant * r * r * (particleDensity - environmentDensity) * gravity * (1.f - std::pow(capacity/maxCapacity, capacityHinderingExponent)));
    return acceleration;
}

Vector3 getParticleVelocity() {
    return Vector3();
}

//Vector3 getExternalForce(Vector3& position, float gravity, float particleMass, float particleVolume, float particleDensity, float environmentDensity, GridV3& flowfieldValues, Vector3& gravityDirection, float capacity, float maxCapacity) {
Vector3 getExternalForce(Vector3& position, float gravity, float particleMass, float particleVolume, float particleDensity, float environmentDensity, const Vector3& flowfieldValues, Vector3& gravityDirection, float capacity, float maxCapacity) {
    Vector3 gravityForce = getGravityForce(position, gravity, particleMass, particleVolume, particleDensity, environmentDensity, flowfieldValues, gravityDirection);
    Vector3 settlingForce = Vector3(); //getSettlingForce(particleDensity, environmentDensity, gravity, capacity, maxCapacity, gravityDirection);
    return gravityForce + settlingForce;
}
//void leapfrogVerlet(Vector3& position, Vector3& velocity, Vector3& acceleration, float dt, GridV3& flowfield, bool applyFlow) {
void leapfrogVerlet(Vector3& position, Vector3& velocity, Vector3& acceleration, float dt, const Vector3& flowfield, bool applyFlow) {
    // Verlet : x_n+1 = 2 * x_n - x_n-1 + acc * dt^2
    Vector3 oldPos = (position - velocity * dt); // Yeah... it's more an implicit Euler integration...
//    Vector3 usedVelocity = position - oldPos; // (Should be "position - oldPos;")
//    Vector3 v_halfNext = getParticleVelocity(position) + acceleration * dt;
//    position += v_halfNext * dt;
    Vector3 displacement = (position - oldPos) + 0.5f * acceleration * dt * dt;
    if (applyFlow)
        displacement += flowfield * dt;
    //    displacement += flowfield.at(position) * dt;
//    std::cout << displacement << " " << flowfield.at(position) << std::endl;
    Vector3 nextPosition = position + displacement.maxMagnitude(1.f); //(2.f * position - oldPos) + acceleration * dt * dt;
    velocity = nextPosition - position;
    position = nextPosition;
    acceleration *= 0;
}

Vector3 computeParticleForces(ErosionParticle& particle, EnvironmentProperty& env, MaterialProperty& mat)
{
    Vector3 Fg = particle.mass * particle.volume * env.gravity; // Gravity
    Vector3 Fb = -particle.volume * env.density * env.gravity; // Buoyancy
    Vector3 Fd = -6.f * M_PI * env.viscosity * particle.radius * particle.velocity; // Drag

    return Fg + Fb + Fd;
}
void updateParticlePosition(ErosionParticle& particle, EnvironmentProperty& env, MaterialProperty& mat) {
    // Verlet : x_n+1 = 2 * x_n - x_n-1 + acc * dt^2
    float dt = 0.001f;
//    float dt2 = dt * .5f;

    // Euler :
    particle.velocity += (computeParticleForces(particle, env, mat)) * dt;
    particle.pos += particle.velocity;
    particle.dir = particle.velocity.normalized();
    particle.force *= 0.f;
}

std::pair<float, float> computeErosionDeposition(ErosionParticle& particle, EnvironmentProperty& env, MaterialProperty& mat, float erosionFactor, float depositFactor) {
    // Erosion :
    // Constants
    float shearingStressConstantK = 1.f;
    float shearingRatePower = .5f;
    float erosionPowerValue = 1.f;
    float l = particle.radius * 0.002f;

    // Computations
    float relativeVelocity = (1.f - std::abs(particle.dir.dot(mat.normal))); // velocity relative to the surface
    float shearRate = relativeVelocity / l;
    float shear = shearingStressConstantK * std::pow(shearRate, shearingRatePower);
    float amountToErode = erosionFactor * std::pow(std::max(0.f, shear - mat.criticalShearStress), erosionPowerValue) * (1.f - mat.resistance);
    amountToErode = std::min(amountToErode, particle.maxCapacity - particle.capacity);

    // Deposition :
    // Constants
    float hinderedPower = 1.f;
    float densitiesRatio = particle.density / mat.density;

    // Computations
    float hinderedFunction = 1.f - std::pow(particle.capacity / particle.maxCapacity, hinderedPower);
    Vector3 settlingVelocity = (2.f/9.f) * particle.radius * particle.radius * ((particle.density - env.density) / env.viscosity) * env.gravity * hinderedFunction;
    float amountToDeposit = std::min(particle.capacity, depositFactor * densitiesRatio);

    // Here, maybe we could change the settling depending on the normal, but meh
    return {amountToErode, amountToDeposit};
}

void initializeParticle(ErosionParticle& particle, Vector3& position, Vector3& velocity, float radius, float density, float initialCapacity, float maxCapacity)
{
    particle.pos = position;
    particle.dir = velocity;
    particle.density = density;
    particle.capacity = initialCapacity;
    particle.maxCapacity = maxCapacity;
    particle.force = Vector3();
    particle.volume = 1.f;
    particle.radius = radius;
    particle.volume = .74 * M_PI * particle.radius * particle.radius;
    particle.mass = particle.density * particle.volume;
}
/*
std::tuple<std::vector<BSpline>, int, int>
UnderwaterErosion::Apply(EROSION_APPLIED applyOn,
                         TerrainModel* terrain,
                         SpacePartitioning& boundariesTree,
                         float &particleSimulationTime, float &terrainModifTime,
                         Vector3 startingPoint,
                         Vector3 originalDirection,
                         float randomnessFactor,
                         bool fallFromSky,
                         float gravity,
                         float bouncingCoefficient,
                         float bounciness,
                         float minSpeed,
                         float maxSpeed,
                         float maxCapacityFactor,
                         float erosion,
                         float deposit,
                         float matterDensity,
                         float materialImpact,
                         float airFlowfieldRotation,
                         float waterFlowfieldRotation,
                         float airForce,
                         float waterForce,
                         float dt,
                         float shearingStressConstantK,
                         float shearingRatePower,
                         float erosionPowerValue,
                         float criticalShearValue,
                         std::vector<std::pair<Vector3, Vector3> > posAndDirs,
//                         std::function<Vector3(Vector3)> flowfieldFunction)
                         FLOWFIELD_TYPE flowType,
                         GridV3 waterFlow,
                         GridV3 airFlow,
                         DENSITY_TYPE densityUsed,
                         GridF densityMap,
                         float initialCapacity,
                         FluidSimType fluidSimType)
{

    VoxelGrid* asVoxels = dynamic_cast<VoxelGrid*>(terrain);
    Heightmap* asHeightmap = dynamic_cast<Heightmap*>(terrain);
    ImplicitPatch* asImplicit = dynamic_cast<ImplicitPatch*>(terrain);
    LayerBasedGrid* asLayers = dynamic_cast<LayerBasedGrid*>(terrain);

    Vector3 terrainSize = voxelGrid->getDimensions(); //terrain->getDimensions();
    float starting_distance = pow(terrainSize.maxComp()/2.f, 2);
    starting_distance = sqrt(3 * starting_distance); // same as sqrt(x+y+z)
    starting_distance *= 2.0; // Leave a little bit of gap

    if (densityMap.size() > 0) {
        densityMap.raiseErrorOnBadCoord = false;
        densityMap.returned_value_on_outside = RETURN_VALUE_ON_OUTSIDE::MIRROR_VALUE;
    }
    // Hydraulic erosion parameters
    float strengthValue = this->maxRockStrength;
    float maxCapacity = 1.f * strengthValue * maxCapacityFactor;
    float erosionFactor = .01f * strengthValue * erosion;
    float depositFactor = .01f * strengthValue * deposit;

    float particleSize = this->maxRockSize;
    float radius = particleSize * .5f;

    GridF initialMapValues;
    if (asVoxels) {
        initialMapValues = asVoxels->getVoxelValues();
    } else if (asHeightmap) {
        initialMapValues = asHeightmap->getHeights();
    } else if (asImplicit) {
        asImplicit->_cached = false;
        initialMapValues = asImplicit->getVoxelized(terrainSize);
    } else if (asLayers) {
        initialMapValues = asLayers->voxelize(terrainSize.z, 3.f);
    }

    initialMapValues.raiseErrorOnBadCoord = false;
    initialMapValues.returned_value_on_outside = RETURN_VALUE_ON_OUTSIDE::REPEAT_VALUE;

    // Use normals from initial map, it shouldn't change too much...
    GridV3 normals = initialMapValues.gradient(); //initialMapValues.binarize().toDistanceMap().gradient();
    normals.raiseErrorOnBadCoord = false;
    normals.returned_value_on_outside = RETURN_VALUE_ON_OUTSIDE::REPEAT_VALUE;

    if (asHeightmap) {
        normals = asHeightmap->getNormals();
//        normals += GridV3::random(normals.sizeX, normals.sizeY, normals.sizeZ) * .1f;
//        normals = -normals + Vector3(0, 0, 1); // Add the Z-component for heightmaps
    }

    for (auto& n : normals)
        n.normalize();

    GridF modifications(initialMapValues.getDimensions());

    GridV3 gravityfieldValues = GridV3(terrainSize, Vector3(0, 0, -gravity));
    gravityfieldValues.raiseErrorOnBadCoord = false;
    gravityfieldValues.defaultValueOnBadCoord = Vector3(0, 0, -gravity);

    GridF environmentalDensities = voxelGrid->getEnvironmentalDensities(); // Here also
    GridV3 flowfieldValues = GridV3(environmentalDensities.getDimensions());
    if (flowType == FLOWFIELD_TYPE::BASIC) {
        Vector3 airDir = Vector3(0, airForce, 0).rotate(0, 0, (airFlowfieldRotation / 180) * PI);
        Vector3 waterDir = Vector3(0, waterForce, 0).rotate(0, 0, (waterFlowfieldRotation / 180) * PI);
        for (size_t i = 0; i < flowfieldValues.size(); i++) {
//            if (flowfieldValues.getCoordAsVector3(i).x > flowfieldValues.sizeX / 2) continue;
            if (environmentalDensities.at(i) < 100) { // In the air
                flowfieldValues.at(i) = airDir;
            } else { // In water
                flowfieldValues.at(i) = waterDir;
            }
        }
    } else if (flowType == FLOWFIELD_TYPE::FLOWFIELD_IMAGE) {
        airFlow = airFlow.resize(terrainSize.x, terrainSize.y, 1) * airForce * 10.f;
        waterFlow = waterFlow.resize(terrainSize.x, terrainSize.y, 1) * waterForce * 10.f;
        for (size_t i = 0; i < flowfieldValues.size(); i++) {
            if (environmentalDensities.at(i) < 100) { // In the air
                flowfieldValues.at(i) = airFlow.at(flowfieldValues.getCoordAsVector3(i).xy());
            } else { // In water
                flowfieldValues.at(i) = waterFlow.at(flowfieldValues.getCoordAsVector3(i).xy());
            }
        }
    } else if (flowType == FLOWFIELD_TYPE::FLUID_SIMULATION) {
        flowfieldValues = voxelGrid->getFlowfield(fluidSimType).resize(terrainSize) * airForce;
    }
    flowfieldValues.raiseErrorOnBadCoord = false;
    flowfieldValues.returned_value_on_outside = RETURN_VALUE_ON_OUTSIDE::REPEAT_VALUE;

    std::vector<BSpline> tunnels(rockAmount);
    std::vector<GridF> submodifications(rockAmount, GridF(modifications.getDimensions()));
    std::vector<int> nbPos(rockAmount), nbErosions(rockAmount);
    std::vector<std::vector<std::pair<float, Vector3>>> allErosions(rockAmount);

    // Cache computations of the erosion matrix
    RockErosion::createPrecomputedAttackMask(particleSize);
    RockErosion::createPrecomputedAttackMask(particleSize * 2);
    RockErosion::createPrecomputedAttackMask2D(particleSize);
    RockErosion::createPrecomputedAttackMask2D(particleSize * 2);

    std::vector<ErosionParticle> particles(this->rockAmount);
    Matrix3<EnvironmentProperty> environmentProperties(environmentalDensities.getDimensions());
    Matrix3<MaterialProperty> materialProperties(environmentalDensities.getDimensions());
    for (size_t i = 0; i < environmentProperties.size(); i++) {
        Vector3 coord = environmentProperties.getCoordAsVector3(i);
        EnvironmentProperty& env = environmentProperties[i];
        env.density = environmentalDensities[i];
        env.gravity = Vector3(0, 0, -gravity);
        env.flowfield = flowfieldValues[i];
        env.viscosity = 1.f;

        MaterialProperty& mat = materialProperties[i];
        mat.normal = normals.at(i);
        mat.density = (terrain->checkIsInGround(coord) ? 1.f : 0.f);
        mat.criticalShearStress = 0.f;

        if (densityUsed == DENSITY_TYPE::DENSITY_IMAGE)
            mat.resistance = densityMap.at(coord) * materialImpact;
        else if (densityUsed == DENSITY_TYPE::RANDOM_DENSITY)
            mat.resistance = densityMap.at(coord) * materialImpact;
    }
    environmentProperties.raiseErrorOnBadCoord = false;
    environmentProperties.returned_value_on_outside = RETURN_VALUE_ON_OUTSIDE::REPEAT_VALUE;
    materialProperties.raiseErrorOnBadCoord = false;
    materialProperties.returned_value_on_outside = RETURN_VALUE_ON_OUTSIDE::REPEAT_VALUE;

    auto startTime = std::chrono::system_clock::now();
    float totalCollisionTime = 0.f;
    float totalOtherTime = 0.f;

    #pragma omp parallel for
    for (int i = 0; i < this->rockAmount; i++)
    {
        std::vector<std::pair<float, Vector3>> erosionValuesAndPositions;

        BSpline tunnel;
        float capacity = maxCapacity * initialCapacity;

        float flowfieldInfluence = 1.0;
        int steps = 100 * starting_distance; // An estimation of how many step we need
        auto [pos, dir] = posAndDirs[i];
//        pos = Vector3(36, 56, 50);

        ErosionParticle& particle = particles[i];
        initializeParticle(particle, pos, dir, .5f, matterDensity, capacity, maxCapacity);

        if (terrain->checkIsInGround(particle.pos))
            continue;

//        ErosionParticle backupParticle;
        bool continueSimulation = true;
        bool hasBeenAtLeastOnceInside = false;
        Vector3 lastCollisionPoint(false);
        while (continueSimulation) {
            Vector3 nextPos = particle.pos + (particle.dir * dt);
            float environmentDensity = environmentalDensities.at(nextPos);
            // TODO !!!!
            // Gravity + Buoyancy
            float particleMass = 1.f, particleVolume = .1f;
            float gravityForce = gravity * particleMass;
            float boyancyForce = -environmentDensity * particleVolume; // B = - rho_fluid * V * g
            float gravityCoefficient = particleVolume * gravity * (matterDensity - environmentDensity) * 0.01f;
            Vector3 justFlow = flowfieldValues.at(particle.pos); // (flowfieldValues.at(pos) + flowfieldValues.at(nextPos)) * .5f;
            Vector3 justGravity = (gravityfieldValues.at(particle.pos) * gravityCoefficient); // (gravityfieldValues.at(nextPos) * gravityCoefficient); // .maxMagnitude(2.f);
            Vector3 flowfield = justFlow + justGravity;
            particle.dir += flowfield * flowfieldInfluence * dt;
            if (Vector3::isInBox(nextPos.xy(), -terrainSize.xy(), terrainSize.xy()))
                hasBeenAtLeastOnceInside = true;
            auto collisionPointAndTriangleIndex = boundariesTree.getIntersectionAndTriangleIndex(particle.pos, particle.pos + particle.dir);
            Vector3 collisionPoint;
            size_t intersectingTriangleIndex;
            std::tie(collisionPoint, intersectingTriangleIndex) = collisionPointAndTriangleIndex;
            if (collisionPoint.isValid()) {
                if ((collisionPoint - lastCollisionPoint).norm2() < 1e-3) {
                    steps = -1000;
                }
                Vector3 normal = boundariesTree.triangles[intersectingTriangleIndex].normal;

                // Using SPH formula
                // Erosion part
                float l = particleSize * 0.001f;
                float vRel = particle.dir.norm() * (1.f - std::abs(particle.dir.normalized().dot(normal))); // velocity relative to the surface
                float theta = vRel / l;
                float shear = shearingStressConstantK * std::pow(theta, shearingRatePower);
                float amountToErode = erosionFactor * std::pow(std::max(0.f, shear), erosionPowerValue); // std::pow(std::max(0.f, shear - criticalShearStress), erosionPowerValue)  * (1.f - resistanceValue * materialImpact);
                amountToErode = std::min(amountToErode, particle.maxCapacity - particle.capacity);

//                    // Deposition part
                float densitiesRatio = 1.f; //1000.f / matterDensity; // The ground should have a density, I'll assume the value 1000 for now
                float u = (2.f / 9.f) * particle.radius * particle.radius * (matterDensity - environmentDensity) * gravity * (1.f - (particle.capacity / particle.maxCapacity));
                float amountToDeposit = std::min(particle.capacity, depositFactor * (densitiesRatio * capacity * (particle.capacity / particle.maxCapacity))); //maxCapacity));
//                    float amountToDeposit = std::min(particle.capacity, depositFactor * u);

                if (amountToErode - amountToDeposit != 0) {
                    particle.capacity += (amountToErode - amountToDeposit);
                    erosionValuesAndPositions.push_back({amountToErode - amountToDeposit, nextPos});
                }

                totalCollisionTime += timeIt([&]() {
                    // Continue the rock tracing
                    int maxCollisions = 20;
                    do {
                        maxCollisions --;
                        if (maxCollisions <= 0) {
                            steps = -1000;
                            break;
                        }
    //                        Vector3 previousDir = particle.dir;
                        Vector3 bounce = particle.dir.reflexion(normal);
    //                        Vector3 initialBounce = bounce;
                        bounce -= normal * bounce.dot(normal) * (1.f - bounciness);
                        particle.dir = bounce * bouncingCoefficient;

                        particle.pos = collisionPoint;
                        lastCollisionPoint = collisionPoint;
                        std::tie(collisionPoint, intersectingTriangleIndex) = boundariesTree.getIntersectionAndTriangleIndex(particle.pos, particle.pos + particle.dir, intersectingTriangleIndex);
                    } while (collisionPoint.isValid());
                });
            }
            steps --;
            tunnel.points.push_back(particle.pos);

            particle.pos = particle.pos + (particle.dir); //.clamped(0.f, 1.f);
//            particle.pos = Vector3::wrap(particle.pos, Vector3(0, 0, -100), Vector3(terrain->getSizeX(), terrain->getSizeY(), 1000));
            particle.dir *= 0.99f;
            if ((hasBeenAtLeastOnceInside && nextPos.z < -20) || particle.pos.z < -20 || steps < 0 || particle.dir.norm2() < 1e-4) {
                if ((steps < 0 || particle.dir.norm2() < 1e-3) && depositFactor > 0.f && Vector3::isInBox(particle.pos.xy(), Vector3(), terrain->getDimensions().xy() - Vector3(1, 1))) {
                    Vector3 depositPosition;
                    totalOtherTime += timeIt([&]() {
                        depositPosition = particle.pos.xy() + Vector3(0, 0, terrain->getHeight(particle.pos));
                    });
                    erosionValuesAndPositions.push_back({-particle.capacity, depositPosition});
                }
                continueSimulation = false;
            }
            if (steps < 0)
                continueSimulation = false;
        }
        tunnels[i] = tunnel;
        nbPos[i] = tunnel.points.size();
        nbErosions[i] = erosionValuesAndPositions.size();

        allErosions[i] = erosionValuesAndPositions;
    }
    auto endParticleTime = std::chrono::system_clock::now();

    std::vector<ImplicitNaryOperator*> allNary(allErosions.size());
    if (asImplicit) {
        for (size_t i = 0; i < allNary.size(); i++) {
            allNary[i] = new ImplicitNaryOperator;
        }
    }

//    float summary = 0.f;
    if (asLayers) {
        for (size_t i = 0; i < allErosions.size(); i++) {
            const auto& erosionValuesAndPositions = allErosions[i];
            for (size_t iRock = 0; iRock < erosionValuesAndPositions.size(); iRock++) {
                auto [val, pos] = erosionValuesAndPositions[iRock];
                int size = particleSize;
                val *= 100.f;
                for (int x = -particleSize; x < particleSize; x++) {
                    for (int y = -particleSize; y < particleSize; y++) {
        //                        for (int z = -particleSize; z < particleSize; z++) {
                        if (x*x + y*y < particleSize * particleSize) {
                            float halfHeight = val * std::sqrt(particleSize*particleSize - (x*x + y*y));
                            asLayers->transformLayer(pos.x + x, pos.y + y, pos.z - halfHeight, pos.z + halfHeight, (val > 0 ? TerrainTypes::AIR : TerrainTypes::DIRT));
                        }
        //                        }
                    }
                }
            }
        }
    } else {
        #pragma omp parallel for
        for (size_t i = 0; i < allErosions.size(); i++) {
            const auto& erosionValuesAndPositions = allErosions[i];
            for (size_t iRock = 0; iRock < erosionValuesAndPositions.size(); iRock++) {
                auto [val, pos] = erosionValuesAndPositions[iRock];
//                val = (val < 0 ? -1 : 1);
                int size = particleSize;
                val *= 100.f;
                if (iRock == erosionValuesAndPositions.size() - 1) {
    //                size *= 2.f;
    //                val *= .5f;
                }
    //            summary += val;

                if (asVoxels) {
    //                std::cout << RockErosion(size, val).createPrecomputedAttackMask(size).sum() - val << " errors" << std::endl;
                    RockErosion(size, val).computeErosionMatrix(submodifications[i], pos);
                } else if (asHeightmap) {
                    RockErosion(size, val).computeErosionMatrix2D(submodifications[i], pos);
    //                std::cout << val << " " << submodifications[i].sum() << std::endl;
                } else if (asImplicit) {
                    if (val <= 0.f) continue; // Fits the paper Paris et al. Terrain Amplification with Implicit 3D Features (2019)
                    float dimensions = size * val * 5.f;
                    auto sphere = dynamic_cast<ImplicitPrimitive*>(ImplicitPatch::createPredefinedShape(ImplicitPatch::Sphere, Vector3(dimensions, dimensions, dimensions), val));
                    sphere->material = (val > 0 ? TerrainTypes::AIR : TerrainTypes::DIRT);
                    sphere->dimensions = Vector3(dimensions, dimensions, dimensions);
                    sphere->supportDimensions = sphere->dimensions;
                    sphere->position = pos - sphere->dimensions * .5f;
                    allNary[i]->composables.push_back(sphere);
                }
            }
        }
    }

    for (const auto& sub : submodifications)
        modifications += sub;

    if (densityMap.size() > 0) {
        for (size_t i = 0; i < modifications.size(); i++) {
            modifications[i] = modifications[i] * ((1.f - materialImpact) + (1.f - densityMap[i]) * materialImpact);
        }
    }

    std::cout << "Total erosion : " << modifications.sum() << std::endl;
    std::cout << "Collisions cost " << showTime(totalCollisionTime) << std::endl;
    std::cout << "Other cost " << showTime(totalOtherTime) << std::endl;
    if (asVoxels) {
        for (int x = 0; x < modifications.sizeX; x++)
            for (int y = 0; y < modifications.sizeY; y++)
                for (int z = 0; z < 2; z++)
                    modifications.at(x, y, z) = 0;
        asVoxels->applyModification(modifications * 0.5f);
        asVoxels->saveState();
    } else if (asHeightmap) {
        asHeightmap->heights += modifications * 0.5f;
//        std::cout << summary << " " << modifications.sum() << std::endl;
    } else if (asImplicit) {
        ImplicitNaryOperator* totalErosion = new ImplicitNaryOperator;
        for (auto& op : allNary)
            totalErosion->composables.push_back(op);
//        totalErosion->composables.push_back(implicitTerrain->copy());
//        delete implicitTerrain;
//        implicitTerrain->~ImplicitPatch();
//        implicitTerrain = new (implicitTerrain) ImplicitNaryOperator;
        dynamic_cast<ImplicitNaryOperator*>(implicitTerrain)->composables.push_back(totalErosion);
        implicitTerrain->_cached = false;
    } else if (asLayers) {
        // ...
    }

    int positions = 0, erosions = 0;
    for (int i = 0; i < rockAmount; i++) {
        positions += nbPos[i];
        erosions += nbErosions[i];
    }
    auto endModificationTime = std::chrono::system_clock::now();

    particleSimulationTime = std::chrono::duration_cast<std::chrono::milliseconds>(endParticleTime - startTime).count();
    terrainModifTime = std::chrono::duration_cast<std::chrono::milliseconds>(endModificationTime - endParticleTime).count();
    return {tunnels, positions, erosions};
}
*/








std::tuple<std::vector<BSpline>, int, int, std::vector<std::vector<std::pair<float, Vector3>>>>
UnderwaterErosion::Apply(EROSION_APPLIED applyOn,
                         TerrainModel* terrain,
                         SpacePartitioning& boundariesTree,
                         float &particleSimulationTime, float &terrainModifTime,
                         Vector3 startingPoint,
                         Vector3 originalDirection,
                         float randomnessFactor,
                         bool fallFromSky,
                         float gravity,
                         float bouncingCoefficient,
                         float bounciness,
                         float minSpeed,
                         float maxSpeed,
                         float maxCapacityFactor,
                         float erosion,
                         float deposit,
                         float matterDensity,
                         float materialImpact,
                         float airFlowfieldRotation,
                         float waterFlowfieldRotation,
                         float airForce,
                         float waterForce,
                         float dt,
                         float shearingStressConstantK,
                         float shearingRatePower,
                         float erosionPowerValue,
                         float criticalShearValue,
                         std::vector<std::pair<Vector3, Vector3> > posAndDirs,
//                         std::function<Vector3(Vector3)> flowfieldFunction)
                         FLOWFIELD_TYPE flowType,
                         GridV3 waterFlow,
                         GridV3 airFlow,
                         DENSITY_TYPE densityUsed,
                         GridF densityMap,
                         float initialCapacity,
                         FluidSimType fluidSimType,
                         bool wrapPositions, bool applyTheErosion)
{

    VoxelGrid* asVoxels = dynamic_cast<VoxelGrid*>(terrain);
    Heightmap* asHeightmap = dynamic_cast<Heightmap*>(terrain);
    ImplicitPatch* asImplicit = dynamic_cast<ImplicitPatch*>(terrain);
    LayerBasedGrid* asLayers = dynamic_cast<LayerBasedGrid*>(terrain);

    Vector3 terrainSize = voxelGrid->getDimensions(); //terrain->getDimensions();
    float starting_distance = pow(terrainSize.maxComp()/2.f, 2);
    starting_distance = sqrt(3 * starting_distance); // same as sqrt(x+y+z)
    starting_distance *= 2.0; // Leave a little bit of gap

    if (densityMap.size() > 0) {
        densityMap.raiseErrorOnBadCoord = false;
        densityMap.returned_value_on_outside = RETURN_VALUE_ON_OUTSIDE::MIRROR_VALUE;
    }
    // Hydraulic erosion parameters
    float strengthValue = this->maxRockStrength;
    float maxCapacity = 1.f * strengthValue * maxCapacityFactor;
    float erosionFactor = .01f * strengthValue * erosion;
    float depositFactor = .01f * strengthValue * deposit;

    float particleSize = this->maxRockSize;
    float radius = particleSize * .5f;

    GridF modifications((asHeightmap ? Vector3(terrainSize.x, terrainSize.y, 1) : terrainSize));

    Vector3 gravityDefault = Vector3(0, 0, -gravity);
//    auto environmentalDensities = [&](const Vector3& pos) {
//        if (pos.z < 0) return 1000.f;
//        return 1.f;
//    };
//    auto flowfieldValues = [&](const Vector3& pos) {
//        return Vector3();
//    };
//    GridV3 gravityfieldValues = GridV3(terrainSize, Vector3(0, 0, -gravity));
//    gravityfieldValues.raiseErrorOnBadCoord = false;
//    gravityfieldValues.defaultValueOnBadCoord = Vector3(0, 0, -gravity);

    GridF environmentalDensities = voxelGrid->getEnvironmentalDensities(); // Here also
    GridV3 flowfieldValues = GridV3(terrainSize);
    if (flowType == FLOWFIELD_TYPE::BASIC) {
        Vector3 airDir = Vector3(0, airForce, 0).rotate(0, 0, (airFlowfieldRotation / 180) * PI);
        Vector3 waterDir = Vector3(0, waterForce, 0).rotate(0, 0, (waterFlowfieldRotation / 180) * PI);
        for (size_t i = 0; i < flowfieldValues.size(); i++) {
            Vector3 pos = flowfieldValues.getCoordAsVector3(i);
//            if (pos.x > flowfieldValues.sizeX / 2) continue;
            if (environmentalDensities(pos) < 100) { // In the air
                flowfieldValues(pos) = airDir;
            } else { // In water
                flowfieldValues(pos) = waterDir;
            }
        }
    } else if (flowType == FLOWFIELD_TYPE::FLOWFIELD_IMAGE) {
        airFlow = airFlow.resize(terrainSize.x, terrainSize.y, 1) * airForce * 10.f;
        waterFlow = waterFlow.resize(terrainSize.x, terrainSize.y, 1) * waterForce * 10.f;
        for (size_t i = 0; i < flowfieldValues.size(); i++) {
            if (environmentalDensities.at(flowfieldValues.getCoordAsVector3(i)) < 100) { // In the air
                flowfieldValues.at(i) = airFlow.at(flowfieldValues.getCoordAsVector3(i).xy());
            } else { // In water
                flowfieldValues.at(i) = waterFlow.at(flowfieldValues.getCoordAsVector3(i).xy());
            }
        }
    } else if (flowType == FLOWFIELD_TYPE::FLUID_SIMULATION) {
        flowfieldValues = GridV3(voxelGrid->getFlowfield(fluidSimType).resize(terrainSize) * airForce).meanSmooth();
        for (auto& v : flowfieldValues)
            v = v.xy();
    } else if (flowType == FLOWFIELD_TYPE::FLOWFIELD_ENVOBJECTS) {
        flowfieldValues = EnvObject::flowfield.resize(terrainSize).meanSmooth();
        for (auto& v : flowfieldValues)
            v = v.xy();
    }
    flowfieldValues.raiseErrorOnBadCoord = false;
    flowfieldValues.returned_value_on_outside = RETURN_VALUE_ON_OUTSIDE::REPEAT_VALUE;

    std::vector<BSpline> tunnels(rockAmount);
    std::vector<GridF> submodifications(rockAmount, GridF(modifications.getDimensions()));
    std::vector<int> nbPos(rockAmount), nbErosions(rockAmount);
    std::vector<std::vector<std::pair<float, Vector3>>> allErosions(rockAmount);

    // Cache computations of the erosion matrix
    RockErosion::createPrecomputedAttackMask(particleSize);
    RockErosion::createPrecomputedAttackMask(particleSize * 2);
    RockErosion::createPrecomputedAttackMask2D(particleSize);
    RockErosion::createPrecomputedAttackMask2D(particleSize * 2);

    std::vector<ErosionParticle> particles(this->rockAmount);

    auto startTime = std::chrono::system_clock::now();
    float totalCollisionTime = 0.f;
    float totalOtherTime = 0.f;

//    rockAmount = 1;
//    posAndDirs[0] = {Vector3(12, 34, 5), Vector3()};

    #pragma omp parallel for
    for (int i = 0; i < this->rockAmount; i++)
    {
        bool rolling = false;
        std::vector<std::pair<float, Vector3>> erosionValuesAndPositions;

        BSpline tunnel;
        float capacity = maxCapacity * initialCapacity;

        float flowfieldInfluence = 1.0;
        int steps = 500 / dt; // An estimation of how many step we need
        auto [initialPos, initialDir] = posAndDirs[i];

        ErosionParticle& particle = particles[i];
        initializeParticle(particle, initialPos, initialDir, .5f, matterDensity, capacity, maxCapacity);

        if (terrain->checkIsInGround(particle.pos))
            continue;

//        ErosionParticle backupParticle;
        bool continueSimulation = true;
        bool hasBeenAtLeastOnceInside = false;
        Vector3 lastCollisionPoint(false);
        int lastBouncingTime = 10000;
        size_t intersectingTriangleIndex = -1;
        std::vector<Vector3> lastCollisions;
        while (continueSimulation) {
            Vector3 nextPos = particle.pos + (particle.dir * dt);
            float environmentDensity = environmentalDensities.at(particle.pos);
            float particleVolume = .1f;
            float gravityCoefficient = particleVolume * gravity * (matterDensity - environmentDensity) * 0.01f;
//            float gravityCoefficient = 0.f;
            Vector3 justFlow = flowfieldValues(particle.pos); // (flowfieldValues.at(pos) + flowfieldValues.at(nextPos)) * .5f;
            Vector3 justGravity = gravityDefault * gravityCoefficient;
//            Vector3 justGravity = (gravityfieldValues.at(particle.pos) * gravityCoefficient); // (gravityfieldValues.at(nextPos) * gravityCoefficient); // .maxMagnitude(2.f);
//            std::cout << "Z=" << particle.pos.z << " -> flow " << justFlow << " -- gravity: " << justGravity << "(" << gravityDefault << " * " << gravityCoefficient << ") density = " << environmentDensity << " (" << environmentalDensities << ")" << std::endl;
            Vector3 flowfield = justFlow + justGravity;
//            if (!rolling)
                particle.dir += flowfield * flowfieldInfluence * dt;

            particle.dir.maxMagnitude(maxSpeed);
            if (Vector3::isInBox(nextPos.xy(), -terrainSize.xy(), terrainSize.xy()))
                hasBeenAtLeastOnceInside = true;
            auto collisionPointAndTriangleIndex = boundariesTree.getIntersectionAndTriangleIndex(particle.pos, particle.pos + particle.dir, (rolling ? intersectingTriangleIndex : -1));
            Vector3 collisionPoint;
            std::tie(collisionPoint, intersectingTriangleIndex) = collisionPointAndTriangleIndex;
            if (rolling || collisionPoint.isValid()) {
//                steps = std::min(steps, 500);

                if (steps < 0)
                    continueSimulation = false;

                bool justStartedRolling = false;

                Vector3 normal;
                float vRel;

                if (!rolling && lastBouncingTime < 3 && collisionPoint.isValid()) {
                    //steps = -1000;
                    rolling = true;
                    justStartedRolling = true;
//                    particle.pos.z += .1f;
                } else if (rolling && collisionPoint.isValid()) {
                    rolling = false;
                    lastCollisions.push_back(collisionPoint);
                    if (lastCollisions.size() > 5) {
                        lastCollisions.erase(lastCollisions.begin());
                        Vector3 mean;
                        for (const auto& p : lastCollisions)
                            mean += p;
                        mean /= float(lastCollisions.size());
                        float error = 0.f;
                        for (const auto& p : lastCollisions)
                            error += (p - mean).norm2();
//                        if (error / float(lastCollisions.size()) < 1e-0)
//                            continueSimulation = false;
                    }

                }


                if (rolling && !justStartedRolling) {
                    vRel = particle.dir.norm();
                } else {
                    normal = boundariesTree.triangles[intersectingTriangleIndex].normal;
                    float dotNormal = std::abs(particle.dir.normalized().dot(normal));
                    vRel = particle.dir.norm() * (1.f - dotNormal); // velocity relative to the surface

                    if (!continueSimulation && normal.z < 0)
                        continueSimulation = true;
                }

                if (justStartedRolling) {
                    particle.dir = (particle.dir - (normal * particle.dir.dot(normal))).setMag(particle.dir.norm());
                    collisionPointAndTriangleIndex = boundariesTree.getIntersectionAndTriangleIndex(particle.pos, particle.pos + particle.dir, intersectingTriangleIndex);
                    std::tie(collisionPoint, intersectingTriangleIndex) = collisionPointAndTriangleIndex;
                    if (collisionPoint.isValid())
                        normal = boundariesTree.triangles[intersectingTriangleIndex].normal;
//                    std::cout << "CollisionPoint is " << (collisionPoint.isValid() ? "valid" : "NOT valid") << std::endl;
                }

                lastBouncingTime = 0;

                // Using SPH formula
                // Erosion part
                float l = particleSize * 0.001f;
                float theta = vRel / l;
                float shear = shearingStressConstantK * std::pow(theta, shearingRatePower);
                float amountToErode = erosionFactor * std::pow(std::max(0.f, shear)/* - criticalShearValue)*/, erosionPowerValue); // std::pow(std::max(0.f, shear - criticalShearStress), erosionPowerValue)  * (1.f - resistanceValue * materialImpact);
//                if (rolling)
//                    amountToErode = 0.f;
                amountToErode = std::min(amountToErode, particle.maxCapacity - particle.capacity);


//                    // Deposition part
                float densitiesRatio = 1.f; //1000.f / matterDensity; // The ground should have a density, I'll assume the value 1000 for now
//                float u = (2.f / 9.f) * particle.radius * particle.radius * (matterDensity - environmentDensity) * gravity * (1.f - (particle.capacity / particle.maxCapacity));
                float amountToDeposit = std::min(particle.capacity, depositFactor * (densitiesRatio * capacity * (particle.capacity / particle.maxCapacity))); //maxCapacity));
//                    float amountToDeposit = std::min(particle.capacity, depositFactor * u);

                if (amountToErode - amountToDeposit != 0) {
                    particle.capacity += (amountToErode - amountToDeposit);
                    if (nextPos.z >= 1.f) {
                        erosionValuesAndPositions.push_back({amountToErode - amountToDeposit, nextPos});
                    }
                }
                if (collisionPoint.isValid()) {
                    totalCollisionTime += timeIt([&]() {
                        // Continue the rock tracing
                        int maxCollisions = 2;
                        do {
                            maxCollisions --;
                            if (maxCollisions <= 0 || !continueSimulation) {
                                continueSimulation = false;
                                break;
                            }
                            Vector3 bounce = particle.dir.reflexion(normal);
                            bounce -= normal * bounce.dot(normal) * (1.f - bounciness);
                            particle.dir = bounce * bouncingCoefficient;
//                            particle.dir.minMagnitude(.5f);

                            particle.pos = collisionPoint;
                            lastCollisionPoint = collisionPoint;
                            std::tie(collisionPoint, intersectingTriangleIndex) = boundariesTree.getIntersectionAndTriangleIndex(particle.pos, particle.pos + particle.dir, intersectingTriangleIndex);
                        } while (collisionPoint.isValid());
                    });
                } else if (rolling){
                    particle.dir *= bouncingCoefficient;
//                    particle.dir = (particle.dir - (normal * particle.dir.dot(normal))).setMag(particle.dir.norm());
                }
            } else {

            }
            lastBouncingTime ++;
            steps --;
            tunnel.points.push_back(particle.pos);

            particle.pos = particle.pos + (particle.dir * dt);
            if (wrapPositions)
                particle.pos = Vector3::wrap(particle.pos, Vector3(0, 0, -100), Vector3(terrain->getSizeX(), terrain->getSizeY(), 1000));
            particle.dir *= 0.99f;
            if (steps < 0 || (hasBeenAtLeastOnceInside && nextPos.z < -20) || particle.pos.z < -20 || particle.dir.norm2() < 1e-4 || !continueSimulation) {
                if (/*(steps < 0 || particle.dir.norm2() < 1e-3) &&*/ depositFactor > 0.f && Vector3::isInBox(particle.pos/*.xy()*/, Vector3(), terrain->getDimensions()/*.xy() - Vector3(1, 1)*/)) {
                    Vector3 depositPosition;
                    totalOtherTime += timeIt([&]() {
                        depositPosition = particle.pos/*.xy() + Vector3(0, 0, terrain->getHeight(particle.pos))*/;
                    });
                    erosionValuesAndPositions.push_back({-particle.capacity, depositPosition});
                }
                continueSimulation = false;
            }
//            if (steps < 0)
//                continueSimulation = false;
        }
        tunnels[i] = tunnel;
        nbPos[i] = tunnel.points.size();
        nbErosions[i] = erosionValuesAndPositions.size();

        allErosions[i] = erosionValuesAndPositions;
    }
    auto endParticleTime = std::chrono::system_clock::now();

    if (!applyTheErosion) {
        auto flatAllErosions = flattenArray(allErosions);
        int positions = 0;
        for (auto nb : nbPos)
            positions += nb;
        int erosions = flatAllErosions.size();
        return {tunnels, positions, erosions, allErosions};
    }


    std::vector<ImplicitNaryOperator*> allNary(allErosions.size());
    if (asImplicit) {
        for (size_t i = 0; i < allNary.size(); i++) {
            allNary[i] = new ImplicitNaryOperator;
        }
    }

//    float summary = 0.f;
    if (asLayers) {
        for (size_t i = 0; i < allErosions.size(); i++) {
            const auto& erosionValuesAndPositions = allErosions[i];
            Vector3 previousPos(false);
            bool isTheOnlyDepositRock = true;
            for (size_t iRock = 0; iRock < erosionValuesAndPositions.size(); iRock++) {
                bool lastRock = iRock == erosionValuesAndPositions.size() - 1;
                auto [val, pos] = erosionValuesAndPositions[iRock];
                std::cout << val << " * " << std::sqrt(particleSize*particleSize - 0)/particleSize << " = " << std::abs(val) * std::sqrt(particleSize*particleSize - 0)/particleSize << std::endl;
                /*if (!lastRock && previousPos.isValid() && (previousPos - pos).norm2() < 2.f){
                    continue;
                }*/

                if (lastRock)
                    continue;
                previousPos = pos;
                val *= 1.f;
                float max = std::abs(val) * std::sqrt(particleSize*particleSize)/particleSize;
                float radius = max;
                float radiusXY = max * 100.f;
                for (int x = -radiusXY; x < radiusXY; x++) {
                    for (int y = -radiusXY; y < radiusXY; y++) {
                        if (x*x + y*y < radiusXY * radiusXY) {
//                            float halfHeight = std::abs(val) * std::sqrt(particleSize*particleSize - (x*x + y*y))/particleSize;
                            float halfHeight = std::sqrt(radius*radius - (x*x + y*y))/radius;
                            float startZ = std::max(0.f, pos.z - halfHeight);
                            float endZ = pos.z + halfHeight;
                            asLayers->transformLayer(pos.x + x, pos.y + y, startZ, endZ, (val > 0 ? TerrainTypes::AIR : TerrainTypes::SAND));
                        }
                    }
                }
            }
        }
    } else {
        #pragma omp parallel for
        for (size_t i = 0; i < allErosions.size(); i++) {
            const auto& erosionValuesAndPositions = allErosions[i];
            Vector3 previousPos(false);
            for (size_t iRock = 0; iRock < erosionValuesAndPositions.size(); iRock++) {
                auto [val, pos] = erosionValuesAndPositions[iRock];
//                val = (val < 0 ? -1 : 1);
                int size = particleSize;
                val *= 100.f;
                if (iRock == erosionValuesAndPositions.size() - 1) {
    //                size *= 2.f;
    //                val *= .5f;
                }
    //            summary += val;

                if (asVoxels) {
    //                std::cout << RockErosion(size, val).createPrecomputedAttackMask(size).sum() - val << " errors" << std::endl;
                    RockErosion(size, val).computeErosionMatrix(submodifications[i], pos - Vector3(.5f, .5f, .5f));
                } else if (asHeightmap) {
                    RockErosion(size, val).computeErosionMatrix2D(submodifications[i], pos);
    //                std::cout << val << " " << submodifications[i].sum() << std::endl;
                } else if (asImplicit) {
                    if (iRock == erosionValuesAndPositions.size() - 1) {
                        val *= .1f;
                    }
//                    if (val <= 0.f) continue; // Fits the paper Paris et al. Terrain Amplification with Implicit 3D Features (2019)
                    float dimensions = size; //std::abs(size * val);
                    if (dimensions < 1.f) continue;
//                    if (previousPos.isValid() && (previousPos - pos).norm2() < 2.f){
//                        std::cout << "Ignored [" << val << "] because dist = " << (previousPos - pos).norm() << "\n";
//                        continue;
//                    }
                    previousPos = pos;
                    auto sphere = dynamic_cast<ImplicitPrimitive*>(ImplicitPatch::createPredefinedShape(ImplicitPatch::Sphere, Vector3(dimensions, dimensions, dimensions), std::abs(val)*.1f));
                    sphere->material = (val > 0 ? TerrainTypes::AIR : TerrainTypes::DIRT);
                    sphere->dimensions = Vector3(dimensions, dimensions, dimensions);
                    sphere->supportDimensions = sphere->dimensions;
                    sphere->position = pos - sphere->dimensions * .5f;
                    allNary[i]->composables.push_back(sphere);
                }
            }
        }
    }
    std::cout << std::endl;

    for (const auto& sub : submodifications)
        modifications += sub;

    if (densityMap.size() > 0) {
        for (size_t i = 0; i < modifications.size(); i++) {
            modifications[i] = (modifications[i] > 0 ? modifications[i] : modifications[i] * ((1.f - materialImpact) + (1.f - densityMap[i]) * materialImpact));
        }
    }

    std::cout << "Total erosion : " << modifications.sum() << std::endl;
    std::cout << "Collisions cost " << showTime(totalCollisionTime) << std::endl;
    std::cout << "Other cost " << showTime(totalOtherTime) << std::endl;
    if (asVoxels) {
        for (int x = 0; x < modifications.sizeX; x++)
            for (int y = 0; y < modifications.sizeY; y++)
                for (int z = 0; z < 2; z++)
                    modifications.at(x, y, z) = 0;
        asVoxels->applyModification(modifications * 0.5f);
//        asVoxels->limitVoxelValues(1.f);
        asVoxels->saveState();
    } else if (asHeightmap) {
        asHeightmap->heights += modifications * 0.5f;
    } else if (asImplicit) {
        ImplicitNaryOperator* totalErosion = new ImplicitNaryOperator;
        for (auto& op : allNary)
            if (!op->composables.empty())
                totalErosion->composables.push_back(op);
        if (!totalErosion->composables.empty())
        {
    //        totalErosion->composables.push_back(implicitTerrain->copy());
    //        delete implicitTerrain;
    //        implicitTerrain->~ImplicitPatch();
    //        implicitTerrain = new (implicitTerrain) ImplicitNaryOperator;
            dynamic_cast<ImplicitNaryOperator*>(implicitTerrain)->addChild(totalErosion);
            implicitTerrain->_cached = false;
        }
    } else if (asLayers) {
        // ...
    }

    int positions = 0, erosions = 0;
    for (int i = 0; i < rockAmount; i++) {
        positions += nbPos[i];
        erosions += nbErosions[i];
    }
    auto endModificationTime = std::chrono::system_clock::now();

    particleSimulationTime = std::chrono::duration_cast<std::chrono::milliseconds>(endParticleTime - startTime).count();
    terrainModifTime = std::chrono::duration_cast<std::chrono::milliseconds>(endModificationTime - endParticleTime).count();
    return {tunnels, positions, erosions, allErosions};
}











std::vector<Vector3> UnderwaterErosion::CreateTunnel(int numberPoints, bool addingMatter, bool applyChanges,
                                                     KarstHolePredefinedShapes startingShape, KarstHolePredefinedShapes endingShape)
{
    BSpline curve = BSpline(numberPoints); // Random curve
    for (Vector3& coord : curve)
        coord = ((coord + Vector3(1.0, 1.0, 1.0)) / 2.0) * voxelGrid->getDimensions(); //Vector3(grid->sizeX, grid->sizeY, grid->sizeZ);
    return CreateTunnel(curve, addingMatter, true, applyChanges, startingShape, endingShape);
}
std::vector<Vector3> UnderwaterErosion::CreateTunnel(BSpline path, bool addingMatter, bool usingSpheres, bool applyChanges,
                                                     KarstHolePredefinedShapes startingShape, KarstHolePredefinedShapes endingShape)
{
    GridF erosionMatrix(voxelGrid->getDimensions()); //(this->grid->sizeX, this->grid->sizeY, this->grid->sizeZ);
    bool modificationDoesSomething = true;
    BSpline width = BSpline(std::vector<Vector3>({
                                                     Vector3(0.0, 1.0),
                                                     Vector3(0.5, 2.0),
                                                     Vector3(1.0, 1.0)
                                                 }));

    std::vector<Vector3> coords;
    if (usingSpheres) {
        float nb_points_on_path = path.length() / (this->maxRockSize/5.f);
        RockErosion rock(this->maxRockSize, this->maxRockStrength);
        for (const auto& pos : path.getPath(nb_points_on_path)) {
            erosionMatrix = rock.computeErosionMatrix(erosionMatrix, pos);
        }
        if (erosionMatrix.abs().max() < 1.e-6) {
            modificationDoesSomething = false;
        } else {
            erosionMatrix = erosionMatrix.abs();
            erosionMatrix.toDistanceMap();
            erosionMatrix.normalize();
            for (float& m : erosionMatrix) {
                m = interpolation::linear(m, 0.f, 1.0) * this->maxRockStrength * (addingMatter ? 1.f : -1.f);
        //        m = interpolation::quadratic(interpolation::linear(m, 0.f, 5.f)); //(sigmoid(m) - s_0) / (s_1 - s_0);
            }
        }
    } else {
        KarstHole hole(path, this->maxRockSize, this->maxRockSize, startingShape, endingShape);
        GridF holeMatrix;
        Vector3 anchor;
        std::vector<std::vector<Vector3>> triangles = hole.generateMesh();
        std::tie(holeMatrix, anchor) = hole.generateMask(triangles);
        if (holeMatrix.abs().max() == 0) {
            modificationDoesSomething = false;
        } else {
            holeMatrix = holeMatrix.abs().toDistanceMap();
            holeMatrix.normalize();
            for (float& m : holeMatrix) {
//                m = (m > 0 ? 1.0 : 0.0) * -this->maxRockStrength;
                m = interpolation::linear(m, 0.f, 1.0) * -this->maxRockStrength * (addingMatter ? -1.f : 1.f);
        //        m = interpolation::quadratic(interpolation::linear(m, 0.f, 5.f)); //(sigmoid(m) - s_0) / (s_1 - s_0);
            }
            RockErosion rock;
            erosionMatrix = rock.computeErosionMatrix(erosionMatrix, holeMatrix, path.getPoint(0), addingMatter, anchor, true);

            for (const auto& triangle : triangles) {
                coords.push_back(triangle[0]);
                coords.push_back(triangle[1]);
                coords.push_back(triangle[1]);
                coords.push_back(triangle[2]);
                coords.push_back(triangle[2]);
                coords.push_back(triangle[0]);
            }
        }
    }
    if (modificationDoesSomething) {
        voxelGrid->applyModification(erosionMatrix);
    }
//    if (applyChanges)
//        grid->remeshAll();
    return coords;
}

std::vector<std::vector<Vector3> > UnderwaterErosion::CreateMultipleTunnels(std::vector<BSpline> paths, bool addingMatter, bool usingSpheres, bool applyChanges,
                                                                            KarstHolePredefinedShapes startingShape, KarstHolePredefinedShapes endingShape)
{
    GridF erosionMatrix(voxelGrid->getDimensions()); // this->grid->sizeX, this->grid->sizeY, this->grid->sizeZ);
    BSpline width = BSpline(std::vector<Vector3>({
                                                     Vector3(0.0, 1.0),
                                                     Vector3(0.5, 0.5),
                                                     Vector3(1.0, 1.0)
                                                 }));

//    float resolution = 0.01;
    std::vector<std::vector<Vector3>> allCoords;
    for (BSpline& path : paths) {
        bool modificationDoesSomething = true;
        if (usingSpheres) {
            float nb_points_on_path = path.length() / (float)(this->maxRockSize / 3.f);
            std::vector<Vector3> coords;
            RockErosion rock(this->maxRockSize, this->maxRockStrength);
            for (const auto& pos : path.getPath(nb_points_on_path)) {
                erosionMatrix = rock.computeErosionMatrix(erosionMatrix, pos);
            }
        } else {
            KarstHole hole(path, this->maxRockSize, this->maxRockSize, startingShape, endingShape);
            GridF holeMatrix;
            Vector3 anchor;
            std::vector<std::vector<Vector3>> triangles = hole.generateMesh();
            std::tie(holeMatrix, anchor) = hole.generateMask(triangles);
            if (holeMatrix.abs().max() == 0) {
                modificationDoesSomething = false;
            } else {
                holeMatrix = holeMatrix.abs().toDistanceMap();
                holeMatrix.normalize();
                for (float& m : holeMatrix) {
                    m = interpolation::linear(m, 0.f, 1.0) * this->maxRockStrength * (addingMatter ? 1.f : -1.f);
            //        m = interpolation::quadratic(interpolation::linear(m, 0.f, 5.f)); //(sigmoid(m) - s_0) / (s_1 - s_0);
                }
//                holeMatrix *= -this->maxRockStrength;
                RockErosion rock;
                erosionMatrix = rock.computeErosionMatrix(erosionMatrix, holeMatrix, path.getPoint(0), addingMatter, anchor);

                std::vector<Vector3> coords;
                for (const auto& triangle : triangles) {
                    coords.push_back(triangle[0]);
                    coords.push_back(triangle[1]);
                    coords.push_back(triangle[1]);
                    coords.push_back(triangle[2]);
                    coords.push_back(triangle[2]);
                    coords.push_back(triangle[0]);
                }
                allCoords.push_back(coords);
            }
        }
    }
    voxelGrid->applyModification(erosionMatrix * 20.f);
//    if (applyChanges)
//        grid->remeshAll();
    return allCoords;
}

std::vector<Vector3> UnderwaterErosion::CreateCrack(const Vector3& start, const Vector3& end, bool applyChanges)
{
    float rx = 3.f, ry = 3.f, rz = 3.f;
    Vector3 ratio(rx, ry, rz);
    GridI resizedMap = this->voxelGrid->getVoxelValues().resize(voxelGrid->getDimensions() / ratio).binarize();
//    GridI resizedMap = this->grid->getVoxelValues().resize(this->grid->sizeX / rx, this->grid->sizeY / ry, this->grid->sizeZ / rz).binarize();
    Matrix3Graph graph = Matrix3Graph(resizedMap).computeSurface().randomizeEdges(.5f);
    Vector3 clampedStart = start / ratio;
    clampedStart.x = std::clamp(clampedStart.x, 0.f, resizedMap.sizeX - 1.f);
    clampedStart.y = std::clamp(clampedStart.y, 0.f, resizedMap.sizeY - 1.f);
    clampedStart.z = std::clamp(clampedStart.z, 0.f, resizedMap.sizeZ - 1.f);
    Vector3 clampedEnd = end / ratio;
    clampedEnd.x = std::clamp(clampedEnd.x, 0.f, resizedMap.sizeX - 1.f);
    clampedEnd.y = std::clamp(clampedEnd.y, 0.f, resizedMap.sizeY - 1.f);
    clampedEnd.z = std::clamp(clampedEnd.z, 0.f, resizedMap.sizeZ - 1.f);
    std::vector<Vector3> path = graph.shortestPath(clampedStart, clampedEnd);

    ratio = ((start / clampedStart.rounded()) + (end / clampedEnd.rounded())) / 2.f; // To be continued...
    for (Vector3& point : path)
        point *= ratio;
    std::vector<Vector3> meshPath;
    for (size_t i = 0; i < path.size() - 1; i++) {
        meshPath.push_back(path[i]);
        meshPath.push_back(path[i + 1]);
    }
    return this->CreateTunnel(BSpline(path).simplifyByRamerDouglasPeucker(0.5f), false, false, applyChanges, KarstHolePredefinedShapes::CRACK, KarstHolePredefinedShapes::CRACK);
    //    return meshPath;
}

std::vector<Vector3> UnderwaterErosion::CreateTunnel(KarstHole &tunnel, bool addingMatter, bool applyChanges)
{
    GridF erosionMatrix(voxelGrid->getDimensions()); //(this->grid->sizeX, this->grid->sizeY, this->grid->sizeZ);
    bool modificationDoesSomething = true;
    BSpline width = BSpline(std::vector<Vector3>({
                                                     Vector3(0.0, 1.0),
                                                     Vector3(0.5, 2.0),
                                                     Vector3(1.0, 1.0)
                                                 }));

    std::vector<Vector3> coords;
    GridF holeMatrix;
    Vector3 anchor;
    std::vector<std::vector<Vector3>> triangles = tunnel.generateMesh();
    std::tie(holeMatrix, anchor) = tunnel.generateMask(triangles);
    if (holeMatrix.abs().max() == 0) {
        modificationDoesSomething = false;
    } else {
        holeMatrix = holeMatrix.abs().toDistanceMap();
        holeMatrix.normalize();
        for (float& m : holeMatrix) {
//                m = (m > 0 ? 1.0 : 0.0) * -this->maxRockStrength;
            m = interpolation::linear(m, 0.f, 1.0) * -this->maxRockStrength * (addingMatter ? -1.f : 1.f);
    //        m = interpolation::quadratic(interpolation::linear(m, 0.f, 5.f)); //(sigmoid(m) - s_0) / (s_1 - s_0);
        }
        holeMatrix = holeMatrix.meanSmooth();
        RockErosion rock;
        erosionMatrix = rock.computeErosionMatrix(erosionMatrix, holeMatrix, tunnel.path.getPoint(0), addingMatter, anchor, true);

        for (const auto& triangle : triangles) {
            coords.push_back(triangle[0]);
            coords.push_back(triangle[1]);
            coords.push_back(triangle[1]);
            coords.push_back(triangle[2]);
            coords.push_back(triangle[2]);
            coords.push_back(triangle[0]);
        }
    }
    if (modificationDoesSomething) {
        voxelGrid->applyModification(erosionMatrix);
    }
//    if (applyChanges)
//        grid->remeshAll();
    return coords;
}

