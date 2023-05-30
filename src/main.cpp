#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

//#define OPENVDB_DLL
#include "Utils/Globals.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <qapplication.h>
#include <iostream>
#include "Interface/Interface.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <string>

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
#endif
    QApplication app(argc, argv);
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

    QGLFormat glFormat;
    glFormat.setVersion(4, 5);
    glFormat.setProfile(QGLFormat::CoreProfile);
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

    ViewerInterface vi;
    vi.show();

    return app.exec();
}
