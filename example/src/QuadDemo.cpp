#include "QuadDemo.h"
#include <chrono>


QuadDemo::QuadDemo(std::string meshName, int numSubdivisions, bool wireframe, bool smooth)
    : _meshName(meshName), _numSubdivisions(numSubdivisions), _wireframe(wireframe), _smooth(smooth)
{

}


void QuadDemo::setup()
{
	ofSetVerticalSync(false);
	ofEnableAlphaBlending();
	ofBackground(255);

    _light1.setPosition(500, 500, 500);
    _light1.setDiffuseColor(ofColor(196, 196, 196));
    
    _light2.setPosition(0, 0, -500);
    _light2.setDiffuseColor(ofColor(128, 128, 128));
    _light2.setDirectional();
    _light2.lookAt(ofVec3f(0, 0, 0), ofVec3f(0, 1, 0));

    if (_meshName == "cube") {
        auto v0 = _quad.addVertex({100.0, -100.0, 100.0});
        auto v1 = _quad.addVertex({100.0, 100.0, 100.0});
        auto v2 = _quad.addVertex({-100.0, 100.0, 100.0});
        auto v3 = _quad.addVertex({-100.0, -100.0, 100.0});

        auto v4 = _quad.addVertex({100.0, -100.0, -100.0});
        auto v5 = _quad.addVertex({-100.0, -100.0, -100.0});
        auto v6 = _quad.addVertex({-100.0, 100.0, -100.0});
        auto v7 = _quad.addVertex({100.0, 100.0, -100.0});

        _quad.addFace(v0, v1, v2, v3);
        _quad.addFace(v4, v5, v6, v7);
        _quad.addFace(v0, v4, v7, v1);
        _quad.addFace(v3, v2, v6, v5);
        _quad.addFace(v1, v7, v6, v2);
        _quad.addFace(v0, v3, v5, v4);
    }
    else if (_meshName == "cube2") {
        _quad.load("cube2.obj");
    }
    else {
        cerr << "ERROR: Mesh " << _meshName << " not found" << endl;
    }

    auto t0 = std::chrono::high_resolution_clock::now();
    _quad = _quad.subdivide(_numSubdivisions);
    auto t1 = std::chrono::high_resolution_clock::now();
    float seconds = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() / 1000000.0;
    cout << "subdivision took " << seconds << " seconds" << endl;
}


void QuadDemo::draw()
{
    ofSetColor(0);
	ofEnableDepthTest();
    ofEnableLighting();

    _light1.enable();
    _light2.enable();
    _material.begin();
    _camera.begin();

    ofRotateY(45.0);
    ofRotateX(15.0);
    ofRotateZ(15.0);
    ofScale(2, 2, 2);

    if (_wireframe) {
        _quad.drawWireframe();
    }
    else {
        _quad.draw(_smooth);
    }
    
    _camera.end();
    _material.end();
    _light1.disable();
    _light2.disable();

    if (ofGetFrameNum() == 1) {
        ofSaveFrame();
    }
}
