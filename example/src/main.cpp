#include "QuadDemo.h"
#include "ofGLProgrammableRenderer.h"
#include "ofAppGLFWWindow.h"
#include "ofAppRunner.h"
#include <vector>

using std::vector;

enum class ArgMode
{
    NONE,
    MESH,
    LEVEL,
    SHADING
};

void printUsage()
{
    cout << "USAGE: example -m mesh_name -l subdivision_level -s wireframe|flat|smooth" << endl;
}

int main(int argc, char **argv)
{
    string meshName = "cube";
    int numSubdivisions = 1;
    bool wireframe = false;
    bool smooth = true;

    // quick n dirty command line argument parsing
    vector<string> arguments(argv, argv + argc);
    ArgMode argMode = ArgMode::NONE;
    for (auto arg: arguments) {
        if (argMode != ArgMode::NONE) {
            if (argMode == ArgMode::MESH) {
                meshName = arg;
            }
            else if (argMode == ArgMode::LEVEL) {
                istringstream iss(arg);
                iss >> numSubdivisions;
                if (!iss.eof()) {
                    cerr << "ERROR: integer required for subdivision level" << endl;
                    return 1;
                }
            }
            else if (argMode == ArgMode::SHADING) {
                if (arg == "wireframe") {
                    wireframe = true;
                    smooth = false;
                }
                else if (arg == "flat") {
                    wireframe = false;
                    smooth = false;
                }
                else if (arg == "smooth") {
                    wireframe = false;
                    smooth = true;
                }
                else {
                    cerr << "ERROR: invalid shading type" << endl;
                    printUsage();
                    return 1;
                }
            }
            argMode = ArgMode::NONE;
        }
        else if (arg == "-m") {
            argMode = ArgMode::MESH;
        }
        else if (arg == "-l") {
            argMode = ArgMode::LEVEL;
        }
        else if (arg == "-s") {
            argMode = ArgMode::SHADING;
        }
        else if (arg == "-h") {
            printUsage();
            return 0;
        }
    }

    if (argMode != ArgMode::NONE) {
        if (argMode == ArgMode::MESH) {
            cerr << "ERROR: missing mesh name" << endl;
        }
        else if (argMode == ArgMode::LEVEL) {
            cerr << "ERROR: missing subdivision level" << endl;
        }
        else if (argMode == ArgMode::SHADING) {
            cerr << "ERROR: missing shading type" << endl;
        }
        printUsage();
        return 1;
    }

    ofAppGLFWWindow window;
    ofSetCurrentRenderer(ofGLProgrammableRenderer::TYPE);
    ofSetOpenGLVersion(4, 4);
    ofSetupOpenGL(&window, 1024, 768, OF_WINDOW);
    ofRunApp(new QuadDemo(meshName, numSubdivisions, wireframe, smooth));

    return 0;
}
