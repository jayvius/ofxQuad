#ifndef QUADDEMO_H
#define QUADDEMO_H


#include "ofMain.h"
#include "../src/Quad.h"


class QuadDemo : public ofBaseApp
{
public:
    QuadDemo(std::string meshName, int numSubdivisions, bool wireframe, bool smooth);
    void setup();
    void draw();

private:
	ofEasyCam _camera;
    ofMaterial _material;
    ofLight _light1;
    ofLight _light2;

    ofx::Quad _quad;

    std::string _meshName;
    int _numSubdivisions;
    bool _wireframe;
    bool _smooth;
};


#endif
