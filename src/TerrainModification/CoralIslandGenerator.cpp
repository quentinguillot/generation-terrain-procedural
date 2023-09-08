#include "CoralIslandGenerator.h"

#include "DataStructure/Matrix3.h"
#include "Graphics/DisplayGraphics.h"

CoralIslandGenerator::CoralIslandGenerator()
{

}

GridF CoralIslandGenerator::generate(GridF heights, float subsidence, float waterLevel, float minCoralHeight,
                                              float maxCoralHeight, float verticalScale, float horizontalScale, float alpha)
{
//    float subsidence = .95f;
//    float waterLevel = .7f;
//    float minCoralHeight = waterLevel - .1f;
//    float maxCoralHeight = waterLevel;

//    float verticalScale = 1.0 / 1.0;
//    float horizontalScale = 1.0 / 1.0;
    float vh = 1.0 / horizontalScale;
    float vv = 1.0 / verticalScale;
//    float alpha = 0.1;

//    GridF heights = heightmap.getHeights();
    float downscale = heights.max();
    heights /= downscale;

    GridF initialCoral(heights.getDimensions());
    GridF lowerBand(heights.getDimensions());

    initialCoral = heights.binarizeBetween(minCoralHeight, maxCoralHeight, true, true);
    GridF dilatedInitialSeed = initialCoral.dilate();
//    lowerBand = ((GridI)dilatedInitialSeed).skeletonize();
    for (size_t i = 0; i < lowerBand.size(); i++)
        lowerBand[i] = (initialCoral[i] != dilatedInitialSeed[i] && heights[i] <= minCoralHeight ? 1.f : lowerBand[i]);
    GridF distanceFromLowerCorals = (1.f - lowerBand).toDistanceMap(true, false); //.normalized();
    distanceFromLowerCorals = distanceFromLowerCorals.max() - distanceFromLowerCorals;
    for (size_t i = 0; i < distanceFromLowerCorals.size(); i++) {
        if (heights[i] > minCoralHeight)
            distanceFromLowerCorals[i] *= -1.f;
    }
//    std::cout << std::endl;
//    distanceFromLowerCorals[heights > minCoralHeight] *= -1
    float distanceWhenLowerCoralTouchesWater = (vh / vv) * (maxCoralHeight - minCoralHeight); // / verticalScale;

    GridI heightAboveWater = heights.binarize(waterLevel, true, true);
    GridI distanceIsSmall = distanceFromLowerCorals.binarize(distanceWhenLowerCoralTouchesWater, false, true);
    GridI heightLowerThanMaxCoral = heights.binarize(maxCoralHeight, false, true);
    GridI heightAboveLowBand = heights.binarize(minCoralHeight, true, true);
    GridI heightAboveHighBand = heights.binarize(maxCoralHeight, true, true);


    GridF insideCorals(heights.getDimensions());
    for (size_t i = 0; i < insideCorals.size(); i++)
        if (heightAboveLowBand[i]) //if (heightAboveWater[i] || (distanceIsSmall[i] && !heightAboveLowBand[i]))
            insideCorals[i] = 1.f;
    GridF aFactorIHaveToCheck = maxCoralHeight - (1.f - (distanceFromLowerCorals.abs() / (distanceFromLowerCorals.abs() * insideCorals).max())) * (maxCoralHeight - minCoralHeight) * alpha;
//    GridF aFactorIHaveToCheck = waterLevel - (1.f - ((distanceWhenLowerCoralTouchesWater - distanceFromLowerCorals) / distanceWhenLowerCoralTouchesWater) * alpha);
    for (size_t i = 0; i < insideCorals.size(); i++)
        insideCorals[i] *= aFactorIHaveToCheck[i];
//    insideCorals *= waterLevel;

    GridF outsideCorals(heights.getDimensions());
    for (size_t i = 0; i < outsideCorals.size(); i++)
        if (!heightAboveLowBand[i]) //if (distanceFromLowerCorals[i] >= distanceWhenLowerCoralTouchesWater)
            outsideCorals[i] = 1.f;
    GridF aFactorIHaveToCheck2 = (maxCoralHeight - (1.f - (distanceFromLowerCorals.abs() / (distanceFromLowerCorals.abs() * insideCorals).max()) * alpha));
//    GridF aFactorIHaveToCheck2 = waterLevel + (distanceWhenLowerCoralTouchesWater - distanceFromLowerCorals) / (distanceWhenLowerCoralTouchesWater * 5.f);
    for (size_t i = 0; i < insideCorals.size(); i++)
        outsideCorals[i] *= aFactorIHaveToCheck2[i];

//    Plotter::getInstance()->reset();
//    Plotter::getInstance()->addImage(1.f - (distanceFromLowerCorals.abs() / (distanceFromLowerCorals.abs() * insideCorals).max()), true);
//    Plotter::getInstance()->show();
//    Plotter::getInstance()->draw();

    GridF finalMap = GridF::max(GridF::max(insideCorals, outsideCorals)/* * clamp(std::pow(1.f - subsidence, .5f) + .8f, 0.f, 1.f)*/, heights * subsidence);
//= np.maximum(np.maximum(insideCorals, outsideCorals) * clamp((1-subsidence)**0.5 + .8, 0, 1), heights * subsidence)
    return finalMap * downscale;
}

std::vector<EnvObject*> CoralIslandGenerator::envObjsFromFeatureMap(const GridV3& img)
{
    std::map<std::tuple<int, int, int>, std::string> colorToFeature = {
        {{255,   0,   0}, "abyss"},
        {{  0,   0, 255}, "reef"},
        {{  0, 255, 255}, "lagoon"},
        {{  0, 255,   0}, "beach"},
        {{255, 255,   0}, "island"}
    };
    std::map<std::string, GridI> featureAreas;
    for (auto& [_, name] : colorToFeature)
        featureAreas[name] = GridI(img.getDimensions());

    for (size_t i = 0; i < img.size(); i++) {
        const auto& pix = img[i];
        featureAreas[colorToFeature[{pix.x, pix.y, pix.z}]][i] = 1;
    }

    auto reefs = ((Matrix3<int>)featureAreas["reef"]).skeletonizeToBSplines();
    for (auto& curve : reefs) {
        curve = curve.resamplePoints().simplifyByRamerDouglasPeucker(5.f);
        for (auto& p : curve)
            p *= .5f;
    }

    for (auto& [name, area] : featureAreas) {
        area = area.fillHoles(true);
    }

    std::vector<EnvObject*> objects;
    for (auto& reef : reefs) {
        if (reef.length() < 30) continue;
        EnvCurve* envReef = dynamic_cast<EnvCurve*>(EnvObject::instantiate("reef"));
        envReef->curve = reef;
        objects.push_back(envReef);
    }
    /*
    auto reefContours = featureAreas["reef"].findContoursAsCurves();
    for (auto& curve : reefContours) {
        EnvCurve* frontReef = dynamic_cast<EnvCurve*>(EnvObject::instantiate("frontreef"));
        frontReef->curve = curve;
        objects.push_back(frontReef);
    }
    */
    auto lagoonContours = featureAreas["lagoon"].findContoursAsCurves();
    for (auto& curve : lagoonContours) {
        for (auto& p : curve)
            p *= .5f;
        ShapeCurve simplifiedCurve = curve.simplifyByRamerDouglasPeucker(5.f);
        simplifiedCurve.resamplePoints(simplifiedCurve.size() * 4);
        if (simplifiedCurve.computeArea() < 5.f) continue;
        EnvArea* lagoon = dynamic_cast<EnvArea*>(EnvObject::instantiate("lagoon"));
        lagoon->area = simplifiedCurve;
        objects.push_back(lagoon);
//        Plotter::getInstance()->addPlot(curve.points);
//        Plotter::getInstance()->addPlot(simplifiedCurve.points, "", Qt::red);
    }
//    Plotter::getInstance()->exec();
    /*
    for (auto curve : featureAreas["island"].findContoursAsCurves()) {
        for (auto& p : curve)
            p *= .5f;
        EnvArea* island = dynamic_cast<EnvArea*>(EnvObject::instantiate("island"));
        island->area = curve;
        objects.push_back(island);
    }

    for (auto curve : featureAreas["beach"].findContoursAsCurves()) {
        for (auto& p : curve)
            p *= .5f;
        EnvArea* beach = dynamic_cast<EnvArea*>(EnvObject::instantiate("beach"));
        beach->area = curve;
        objects.push_back(beach);
    }*/
    return objects;
}
