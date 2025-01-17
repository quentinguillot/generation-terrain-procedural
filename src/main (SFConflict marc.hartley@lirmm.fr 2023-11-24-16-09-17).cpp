#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION

#undef interface

//#define OPENVDB_DLL
#include "Utils/Globals.h"
//#include "sim-fluid-loganzartman/Game.hpp"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <qapplication.h>
#include <iostream>
#include "Interface/Interface.h"
#include "EnvObject/EnvObject.h"
#include "EnvObject/ExpressionParser.h"
#include "FluidSimulation/OpenFoamParser.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <string>

using namespace std;

int main(int argc, char *argv[])
{
//    ExpressionParser parser;
//    std::string formula = "d2({10 0 1}) * -1";

//    bool check = parser.validate(formula);

//    std::cout << check << std::endl;
//    std::cout << "= " << parser.parse(formula)({}) << std::endl;

//    return 0;

    /*ExpressionParser parser;
//    auto myFunction = parser.parse("32 + d(p cross q)", {{"p", Vector3()}, {"q", Vector3()}});
    Vector3 testP, testQ;
    bool check1 = parser.validate("32 + d(p cross q)", {{"p", testP}, {"q", testQ}});
    bool check2 = parser.validate("32 + d(p cross q)", {{"p", testP}});
    bool check3 = parser.validate("32 + d(p cross q)>", {{"p", testP}, {"q", testQ}});
    bool check4 = parser.validate("32 + d(<p cross q)", {{"p", testP}, {"q", testQ}});

    testP = Vector3(1, 10, 0);
    testQ = Vector3(10, 0, 0);

    std::cout << check1 << " => " << (check1 ? std::to_string(parser.parse("32 + d(p cross q)", {{"p", testP}, {"q", testQ}})({{"p", testP}, {"q", testQ}})) : "Not passed") << std::endl;
    std::cout << check2 << " => " << (check2 ? std::to_string(parser.parse("32 + d(p cross q)", {{"p", testP}})({{"p", testQ}, {"q", testP}})) : "Not passed") << std::endl;
    std::cout << check3 << " => " << (check3 ? std::to_string(parser.parse("32 + d(p cross q)>", {{"p", testP}, {"q", testQ}})({{"p", testP}, {"q", testQ}})) : "Not passed") << std::endl;
    std::cout << check4 << " => " << (check4 ? std::to_string(parser.parse("32 + d(<p cross q)", {{"p", testP}, {"q", testQ}})({{"p", testP}, {"q", testQ}})) : "Not passed") << std::endl;
    return 0;*/
    /*while (true) {
        std::string formula;
        std::string vecAsString;
        std::cout << "f(x) = " << std::flush;
        getline(std::cin, formula);
        std::cout << "p = " << std::flush;
        getline(std::cin, vecAsString);
        std::vector<std::string> splitted = split(vecAsString, " ");
        Vector3 p(std::stof(splitted[0]), std::stof(splitted[1]), std::stof(splitted[2]));
        std::cout << "f(" << p << ") = " << parser.parse(formula, {{"p", p}})(p) << std::endl;
    }*/
    /*BSpline spline({
                       Vector3(0, 10, 0),
                       Vector3(1, 10, 0),
                       Vector3(100, 1, 0),
                       Vector3(100, 50, 0),
                       Vector3(0, 100, 0),
                       Vector3(0, 0, 0),
                   });
//    spline.resamplePoints();
    GridI grid(105, 105, 1, 0);
    grid.iterateParallel([&](const Vector3& pos) {
        if (spline.estimateDistanceFrom(pos) < 4.f) grid(pos) = 1;
    });

    std::cout << grid.displayAsPlot() << "\n" << spline << std::endl;
    return 0;*/


    /*
    GridI grid({
                   {0, 1, 1, 0, 0, 0, 0, 0, 0},
                   {0, 1, 1, 0, 0, 0, 0, 0, 0},
                   {0, 1, 1, 0, 0, 1, 0, 0, 0},
                   {0, 0, 1, 1, 1, 1, 0, 0, 0},
                   {0, 0, 1, 1, 1, 1, 1, 0, 0},
                   {0, 0, 1, 1, 1, 1, 1, 1, 0},
                   {0, 0, 1, 1, 1, 1, 1, 1, 0},
                   {0, 0, 0, 1, 1, 1, 1, 1, 0},
                   {0, 0, 0, 0, 1, 1, 1, 0, 0},
                   {0, 0, 0, 0, 0, 0, 0, 0, 0}
               });

    std::vector<ShapeCurve> contours = grid.findContoursAsCurves();
    grid = GridI(grid.getDimensions() * Vector3(2.f, 2.f, 1.f));
    for (size_t iCurve = 0; iCurve < contours.size(); iCurve++) {
        auto& contour = contours[iCurve];
        contour = contour.simplifyByRamerDouglasPeucker(.2f);
        for (size_t i = 0; i < contour.size(); i++) {
            grid(contour[i] * 2.f) = iCurve + 1;
        }
    }
    std::cout << grid.displayValues() << std::endl;

    for (size_t iCurve = 0; iCurve < contours.size(); iCurve++) {
        auto& contour = contours[iCurve].scale(2.f);
        std::cout << contour << std::endl;
    }
    return 0;
    */
    /*
    Vector3 dims(1000, 1000, 1);
    GridF mask = GridF::random(dims);
    GridF grid = GridF::random(dims);
    GridF res(dims);
    GridF res2(dims);

    float timeParallel = timeIt([&]() {
        for (int iter = 0; iter < 1000; iter++) {
            FastNoiseLite noise(iter);
            res = GridF::fbmNoise1D(noise, dims.x, dims.y, dims.z);
//#pragma omp parallel for
//            for (size_t i = 0; i < grid.size(); i++) {
//                res[i] = mask[i] + grid[i];
//            }
//            grid.convolution(mask);
//            grid.iterateParallel([&](float x, float y, float z) {
//                grid(x, y, z) = x + y + z;
//            });
        }
    });
    float timeSeq = timeIt([&]() {
        for (int iter = 0; iter < 1000; iter++) {
//            res2 = mask + grid;
//            for (size_t i = 0; i < grid.size(); i++) {
//                res[i] = mask[i] + grid[i];
//            }
//            grid.convolutionSlow(mask);
//            grid.iterate([&](float x, float y, float z) {
//                grid(x, y, z) = x + y + z;
//            });
        }
    });
//    std::cout << grid.displayAsPlot() << std::endl;
    std::cout << "Parallel: " << showTime(timeParallel) << "\nSequential: " << showTime(timeSeq) << std::endl;
    std::cout << "Check: " << (res == res2 ? "OK" : "Not OK!!!") << std::endl;
    return 0;
    */
    /*std::string initialPath = "erosionsTests";
    std::string destFolder = "asOBJ";
    std::string finalPath = initialPath + "/" + destFolder;
    std::vector<std::string> filenames;
    QDirIterator it(QString::fromStdString(initialPath), QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString dir = it.next();
        filenames.push_back(dir.toStdString());
    }
    size_t nbFiles = filenames.size();

    if (!checkPathExists(finalPath)) {
        makedir(finalPath);
    }
//    std::vector<Mesh> meshes(nbFiles);
    std::cout << showTime(timeIt([&]() {
//    #pragma omp parallel for
        for (size_t i = 0; i < nbFiles; i++) {
            std::string dir = filenames[i];
            auto path = split(dir, "/");
            std::string previousName = path.back();
            std::string basename = split(previousName, ".")[0] + ".obj";
            std::string newFilename = finalPath + "/" + basename;

    //        std::cout << dir << " --> " << newFilename << std::endl;
            std::ofstream file(newFilename);
            file << Mesh().fromStl(dir).toOBJ();
            file.close();
        }
    })) << std::endl;
    return 0;*/
/*
    ImplicitPrimitive* primA = new ImplicitPrimitive;
    ImplicitPrimitive* primB = new ImplicitPrimitive;
    ImplicitNaryOperator* nary = new ImplicitNaryOperator;
    ImplicitBinaryOperator* binary = new ImplicitBinaryOperator;
    ImplicitUnaryOperator* unary = new ImplicitUnaryOperator;

    nary->addChild(binary);
    binary->addChild(primA, 0);
    binary->addChild(unary, 1);
    unary->addChild(primA);

    primA->dimensions = Vector3(2, 2, 0);
    primA->position = Vector3(5, 5, 0) - primA->dimensions*.5f;

    primB->dimensions = Vector3(1, 1, 0);
    primB->position = Vector3(10, 10, 0) - primB->dimensions*.5f;

//    unary->scale(Vector3(3, 3, 1));
//    unary->translate(Vector3(1, 0, 0));
//    unary->rotate(0, 0, M_PI / 2.f);

//    std::cout << nary->getBBox() << std::endl;

    Vector3 pos, newPos;

    pos = Vector3(0, 0, 0);
    newPos = primA->getGlobalPositionOf(pos);
    std::cout << "Position " << pos << " becomes " << newPos << std::endl;

    pos = Vector3(3, 0, 0);
    newPos = primA->getGlobalPositionOf(pos);
    std::cout << "Position " << pos << " becomes " << newPos << std::endl;

    pos = Vector3(0, 3, 0);
    newPos = primA->getGlobalPositionOf(pos);
    std::cout << "Position " << pos << " becomes " << newPos << std::endl;

    pos = Vector3(3, 3, 0);
    newPos = primA->getGlobalPositionOf(pos);
    std::cout << "Position " << pos << " becomes " << newPos << std::endl;

    return 0;*/


#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
#endif
    QApplication app(argc, argv);


    QGLFormat glFormat;
    glFormat.setVersion(4, 5);
    glFormat.setProfile(QGLFormat::CompatibilityProfile);
    glFormat.setSampleBuffers(true);
    glFormat.setDefaultFormat(glFormat);
    glFormat.setSwapInterval(1);
    QGLWidget widget(glFormat);
    widget.makeCurrent();

    const QOpenGLContext *context = GlobalsGL::context();

    qDebug() << "Context valid: " << context->isValid();
    qDebug() << "Really used OpenGl: " << context->format().majorVersion() << "." << context->format().minorVersion();
    qDebug() << "OpenGl information: VENDOR:       " << (const char*)glGetString(GL_VENDOR);
    qDebug() << "                    RENDERDER:    " << (const char*)glGetString(GL_RENDERER);
    qDebug() << "                    VERSION:      " << (const char*)glGetString(GL_VERSION);
    qDebug() << "                    GLSL VERSION: " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

/*
    int isize = 64;
    int jsize = 64;
    int ksize = 64;
    float dx = 0.125;
    FluidSimulation fluidsim(isize, jsize, ksize, dx);

    fluidsim.setSurfaceSubdivisionLevel(2);

    float x, y, z;
    fluidsim.getSimulationDimensions(&x, &y, &z);
    fluidsim.addImplicitFluidPoint(x/2, y/2, z/2, 7.0);

    fluidsim.addBodyForce(0.0, -25.0, 0.0);
    fluidsim.initialize();

    float timestep = 1.0 / 30.0;
    for (;;) {
        fluidsim.update(timestep);
    }

    return 0;*/



    /*
    std::cout << timeIt([]() {
        Fluid fluid;
        gfx::Program texture_copy_program;

        fluid.init();
        texture_copy_program.vertex({"screen_quad.vs.glsl"}).fragment({"texture_copy.fs.glsl"}).compile();

    //    fluid.resize(40, 40, 40);

        for (int i = 0; i < 100; i++) {
            fluid.step();
            fluid.ssbo_barrier();

//            if (i % 100 != 0) continue;
//            const auto particles = fluid.particle_ssbo.map_buffer_readonly<Particle>();
            auto grid = fluid.grid_ssbo.map_buffer<GridCell>();

            int count = 0;
            for (int y = fluid.grid_dimensions.y-1; y >= 0; --y) {
                for (int x = 0; x < fluid.grid_dimensions.x; ++x) {
                    for (int z = 0; z < fluid.grid_dimensions.z; ++z) {
                        const int i = fluid.get_grid_index({x, y, z});
                        if (z == 5) std::cout << (grid[i].type == GRID_FLUID ? "O" : (grid[i].type == GRID_SOLID ? "#" : "."));
                        count += (grid[i].type == 2 ? 1 : 0);
                    }
                }
                std::cout << "/n";
            }
            std::cout << "/n" << count << "/n" << std::endl;
        }
    }) << "ms" << std::endl;

    return 0;
    */
    /*Vector3 A = Vector3(1, 0, 0);
    Vector3 B = Vector3(0.5, 1, 1).normalize();
    Vector3 UP = Vector3(0, 0, 1); // A.cross(B);
    Vector3 LEFT = UP.cross(A);

    Vector3 C = Vector3(
                    std::acos(B.y),
                    std::acos(B.z),
                    std::acos(B.x)
                ) * (180.f / 3.141592f);

    std::cout << C << std::endl;
    return 0;*/
/*
    AABBox bbox(Vector3(0, 0, 0), Vector3(2, 2, 1));

    Matrix3<float> M = Matrix3<float>({
                                          {0, 1},
                                          {1, 0}
                                          });
//    std::cout << M.displayValues() << std::endl;

    Matrix3<float> m(10, 10);
    Vector3 ratio = (M.getDimensions() - Vector3(1, 1, 0)) / (bbox.dimensions() - Vector3(1, 1, 0));
    for (int _x = 0; _x < m.sizeX; _x++) {
        for (int _y = 0; _y < m.sizeY; _y++) {
            for (int _z = 0; _z < m.sizeZ; _z++) {
                float x = _x, y = _y, z = _z;
                Vector3 pos(x, y, z);
                Vector3 query = bbox.normalize(pos); // * M.getDimensions(); //pos * ratio;
                query.z = 0;
                auto val = M.interpolate(query);
                m.at(pos) = val;
            }
        }
    }
    std::cout << M.displayValues() << std::endl;
    std::cout << m.displayValues() << std::endl;
    return 0;*/

/*
    ShapeCurve A = ShapeCurve({
                                  Vector3(0, 0.5, 0),
                                  Vector3(0.5, 0, 0),
                                  Vector3(1, 0.1, 0),
                                  Vector3(0.9, 0.2, 0),
                                  Vector3(0.5, 0.5, 0),
                                  Vector3(0.9, 0.8, 0),
                                  Vector3(1, 0.9, 0),
                                  Vector3(0.5, 1, 0)
                              });
    ShapeCurve B = ShapeCurve({
                                  Vector3(2, 0.5, 0),
                                  Vector3(1.5, 0, 0),
                                  Vector3(1, 0.1, 0),
                                  Vector3(1.1, 0.2, 0),
                                  Vector3(1.5, 0.5, 0),
                                  Vector3(1.1, 0.8, 0),
                                  Vector3(1, 0.9, 0),
                                  Vector3(1.5, 1, 0)
                              });
    ShapeCurve AB = merge(A, B);

    Plotter::init();
    Plotter::getInstance()->addPlot(A.closedPath(), "A", Qt::blue);
    Plotter::getInstance()->addPlot(B.closedPath(), "B", Qt::green);
    Plotter::getInstance()->addPlot(AB.closedPath(), "AB", Qt::red);
    Plotter::getInstance()->addScatter(AB.points, "");
    std::cout << A << std::endl;
    std::cout << B << std::endl;
    std::cout << AB << std::endl;
    return Plotter::getInstance()->exec();*/

//    RegularSimplicialComplex grid(10, 10);
//    grid.getNode(2, 3)->value = 0;
//    grid.getNode(1, 3)->value = 0;
//    grid.getNode(1, 4)->value = 0;
//    grid.removeUnavailableLinks();
//    grid.display();

//    return 0;

//    auto g = CombinMap();
//    g.addFace(5, {}, {100, 101, 102, 103, 104});
//    g.addFace(4, {g.root->beta2, g.root->beta2->beta1}, {0});
//    g.addFace(4, {g.root->beta2, g.root->beta2->beta1}, {1});
//    g.debug();
//    Graph<int> G = g.toGraph().forceDrivenPositioning();
//    auto dual = g.dual(g.root->beta1->beta2);
//    dual.debug();
//    G = dual.toGraph().forceDrivenPositioning();
//    return 0;

//    Plotter::get()->addImage(GridF::gaussian(10, 10, 1, 3.f));
//    return Plotter::get()->exec();

    /*
    EnvObject::readFile("saved_maps/primitives.json");

    GridV3& flow = EnvObject::flowfield;
    flow.reset(Vector3(.01f, 0, 0));
    EnvObject::flowImpactFactor = 1.f;

//    EnvObject::sandDeposit.paste(GridF::gaussian(15, 15, 1, 5.f), 20, 42, 0);

    EnvPoint* point = dynamic_cast<EnvPoint*>(EnvObject::instantiate("motu"));
    point->radius = 10;
    point->sandEffect = 1.f;
    point->position = Vector3(10, 50, 0);
    point->flowEffect = Vector3(0, 0, 0);

    EnvCurve* curve = dynamic_cast<EnvCurve*>(EnvObject::instantiate("passe"));
    curve->width = 20.f;
    curve->sandEffect = 0.f;
    curve->curve = BSpline({
                               Vector3(50, 30, 0),
                               Vector3(30, 50, 0),
                               Vector3(50, 70, 0),
                               Vector3(70, 50, 0),
                               Vector3(50, 30, 0)
//                               Vector3(70, 30, 0),
//                               Vector3(70, 70, 0)
                           });
    curve->flowEffect = Vector3(1, 1, 0).normalize();

    auto c1 = curve->clone();
    c1->curve = BSpline({Vector3(20, 0, 0), Vector3(24, 60, 0)});
    auto c2 = c1->clone();
    c2->curve = BSpline({Vector3(50, 100, 0), Vector3(51, 40, 0)});
    auto c3 = c1->clone();
    c3->curve = BSpline({Vector3(70, 0, 0), Vector3(71, 60, 0)});

    EnvObject::instantiatedObjects.pop_back();
    EnvObject::instantiatedObjects.push_back(c1);
    EnvObject::instantiatedObjects.push_back(c2);
    EnvObject::instantiatedObjects.push_back(c3);

    float angle = 0.f;
    QTimer* timer = new QTimer();
    timer->setInterval(50);

    QObject::connect(timer, &QTimer::timeout, [=, &angle]() {
//        angle += 5.f;
        float strength = .05f;
        Vector3 dir = Vector3(cos(deg2rad(angle)) * strength + .1f, sin(deg2rad(angle)) * strength);
        EnvObject::flowfield.reset(dir);
        float t1 = timeIt([&]() {
            EnvObject::applyEffects();
        });
        float t2 = timeIt([&]() {
            Plotter::get()->addImage(EnvObject::sandDeposit, false, true);
//            Plotter::get()->addImage(EnvObject::flowfield, false, true);
            Plotter::get()->draw();
        });
        std::cout << showTime(t1) << " + " << showTime(t2) << " = " << showTime(t1+t2) << " -> " << EnvObject::sandDeposit.sum() << std::endl;
    });
    timer->start();
    return Plotter::get()->exec();
    */
    /*
    GridV3 grid(100, 100, 1);
    BSpline curve({
                      Vector3(0, 0, 0),
                      Vector3(100, 100, 0),
                      Vector3(0, 100, 0),
                      Vector3(100, 0, 0)
                  });
    for (int intensity = 0; intensity < 10; intensity++) {
        for (auto& p : curve)
            p += Vector3::random(intensity * 10.f).xy();
        std::cout << curve.toString() << std::endl;
        grid.reset();
        for (float factor : {1.f, 2.f, 10.f, 50.f, 100.f}) {
            std::cout << "Subdivision x" << factor << " : " << showTime(timeIt([&]() {
                grid.iterateParallel([&](const Vector3& pos) {
                    auto closestTime = curve.estimateClosestTime(pos, 1e-5, factor);
                    float dist = (pos - curve.getPoint(closestTime)).norm2();
                    grid(pos).x = std::max(grid(pos).x, 1.f - std::clamp(dist * .5f , 0.f, 1.f));
                });
            })) << std::endl;
            auto points = curve.getPath(500);
            for (const auto& p : points) {
                grid(p).y = 1;
            }
            Plotter::get()->addImage(grid);
            Plotter::get()->exec();
        }
    }
    return 0;*/

    ViewerInterface vi;
    vi.show();

    return app.exec();
}
