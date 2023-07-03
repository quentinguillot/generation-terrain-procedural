#ifndef CORALISLANDGENERATOR_H
#define CORALISLANDGENERATOR_H

#include "TerrainGen/Heightmap.h"

class CoralIslandGenerator
{
public:
    CoralIslandGenerator();

    static Matrix3<float> generate(Matrix3<float> heights, float subsidence, float waterLevel, float coralMin, float maxCoralHeight, float verticalScale, float horizontalScale, float alpha);
};

#endif // CORALISLANDGENERATOR_H