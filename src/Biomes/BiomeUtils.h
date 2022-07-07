#ifndef BIOMEUTILS_H
#define BIOMEUTILS_H

#include "Biomes/BiomeInstance.h"
#include "Biomes/BiomeModel.h"
#include "Graph/FastPoissonGraph.h"
#include "Utils/Utils.h"
#include "Utils/Voronoi.h"
#include "Utils/json.h"
#include <set>

/*
 Slopes possible:
- mountain
- flat
- curve
- linear
*/
inline std::vector<std::string> biomesNames =
{
    // Biome principaux
    "ile",
    "plage",
    "recif-frangeant",
    "lagon",
    "recif-barriere",
    "profondeurs",

    // Types de sols
    "plaine",

    // Biomes second ordre
    "patate-corail",
    "mur-corail",
    "passe-corail",
    "zone-algues",
    "tranchee",

    // Biomes minimal
    "rocher",
    "corail",
    "algue",

    // Types de coraux
    "corail-plat-horizontal",
    "corail-plat-vertical",
    "corail-mou",
    "corail-boule"
};

int biomeID(std::string biomeName);
std::string biomeName(int ID);
FastPoissonGraph<int> generateHugeBiomesGraphe(std::vector<int> desiredBiomes, Graph<int> adjencyGraph);
Graph<int> subsetToFitMostBiomes(Graph<int> graph, std::vector<std::string> biomesNames);

std::shared_ptr<BiomeInstance> recursivelyCreateBiomeInstance(nlohmann::json json_content, Vector3 biomePosition, ShapeCurve area);

std::shared_ptr<BiomeInstance> generateBiome(Graph<int> biomeGraph);


#endif // BIOMEUTILS_H
