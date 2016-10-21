#include "Quad.h"
#include <memory>
#include <cassert>
#include <iostream>
#include "ofVboMesh.h"
#include <exception>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace ofx;

// Convert two vertex indices into edge key by returning pair of
// vertex indices where first index in pair is greater than second.
EdgeKey makeEdgeKey(VertexID v0, VertexID v1)
{
    if (v0 == -1 || v1 == -1) {
        return {-1, -1};
    }

    if (v0 > v1) {
        auto temp = v0;
        v0 = v1;
        v1 = temp;
    }

    return {v0, v1};
}

Quad::Quad() : _redrawMesh(true)
{

}

Quad::Quad(string objFilename) : _redrawMesh(true)
{
    load(objFilename);
}

// Load mesh data from OBJ file.
// Mesh in OBJ file is restricted to 4 sided polygons.
void Quad::load(std::string objFilename)
{
    ifstream input("./bin/" + objFilename);

    vector<VertexID> vertexIDs;

    string line;
    vector<string> tokens;
    while (getline(input, line)) {
        tokens.clear();
        boost::split(tokens, line, boost::is_any_of(" "));
        if (tokens.size() == 4 && tokens[0] == "v") {
            auto vID = addVertex({stof(tokens[1]), stof(tokens[2]), stof(tokens[3])});
            vertexIDs.push_back(vID);
        }
        else if (tokens.size() == 5 && tokens[0] == "f") {
            auto v0 = vertexIDs[stoi(tokens[1]) - 1];
            auto v1 = vertexIDs[stoi(tokens[2]) - 1];
            auto v2 = vertexIDs[stoi(tokens[3]) - 1];
            auto v3 = vertexIDs[stoi(tokens[4]) - 1];
            addFace(v0, v1, v2, v3);
        }
    }

    _redrawMesh = true;
}

// Given vertex position, add new vertex to mesh
// and return vertex ID of new vertex
VertexID Quad::addVertex(ofVec3f vertex)
{
    // Initialize normal of vertex to all zeros. This will be calculated later.
    // Initialize new vertex ID to -1. This is used internally when doing
    // subdivision surface calculations.
    _vertices.push_back({vertex, {0.0, 0.0, 0.0}, -1});
    _redrawMesh = true;
    return _vertices.size() - 1;
}

// Given four valid vertex IDs, add new face to mesh
// and return face ID of new face
FaceID Quad::addFace(VertexID v0, VertexID v1, VertexID v2, VertexID v3)
{
    _edges.push_back({v0, -1, -1, -1, -1});
    EdgeID edge0 = _edges.size() - 1;

    _edges.push_back({v1, -1, -1, -1, -1});
    EdgeID edge1 = _edges.size() - 1;
    
    _edges.push_back({v2, -1, -1, -1, -1});
    EdgeID edge2 = _edges.size() - 1;
    
    _edges.push_back({v3, -1, -1, -1, -1});
    EdgeID edge3 = _edges.size() - 1;

    _edges[edge0].next = edge1;
    _edges[edge1].next = edge2;
    _edges[edge2].next = edge3;
    _edges[edge3].next = edge0;

    // function for attaching an edge to an adjacent edge, if exists.
    // Given two vertex IDs, search for existing edge with those vertices
    // as endpoints.
    auto attachEdge = [this](EdgeID edge, VertexID a, VertexID b) {
        auto neighborEdge = findEdge(a, b);
        if (neighborEdge == -1) {
            _edgeMap[makeEdgeKey(a, b)] = edge;
        }
        else {
            _edges[neighborEdge].opposite = edge;
            _edges[edge].opposite = neighborEdge;
        }
    };

    // Attach each edge to an adjacent edge with same endpoints, if it exists
    attachEdge(edge0, v0, v1);
    attachEdge(edge1, v1, v2);
    attachEdge(edge2, v2, v3);
    attachEdge(edge3, v3, v0);

    _faces.push_back({edge0, edge1, edge2, edge3, -1});
    FaceID face = _faces.size() - 1;

    _edges[edge0].face = face;
    _edges[edge1].face = face;
    _edges[edge2].face = face;
    _edges[edge3].face = face;

    _redrawMesh = true;
    return face;
}

// Draw mesh using either flat shading or smooth shading
void Quad::draw(bool smoothShading)
{
    // If mesh has changed, build OpenFrameworks tri mesh out of quad mesh
    // and draw tri mesh
    if (_redrawMesh) {
        _mesh.clear();
        calculateNormals();

        for (auto &f: _faces) {
            auto v0 = _vertices[_edges[f.edges[0]].vertex];
            auto v1 = _vertices[_edges[f.edges[1]].vertex];
            auto v2 = _vertices[_edges[f.edges[2]].vertex];
            auto v3 = _vertices[_edges[f.edges[3]].vertex];

            _mesh.addVertex(v0.position);
            _mesh.addVertex(v1.position);
            _mesh.addVertex(v2.position);

            _mesh.addVertex(v0.position);
            _mesh.addVertex(v2.position);
            _mesh.addVertex(v3.position);
            
            if (smoothShading) {
                _mesh.addNormal(v0.normal);
                _mesh.addNormal(v1.normal);
                _mesh.addNormal(v2.normal);
                _mesh.addNormal(v0.normal);
                _mesh.addNormal(v2.normal);
                _mesh.addNormal(v3.normal);
            } else {
                _mesh.addNormal(f.normal);
                _mesh.addNormal(f.normal);
                _mesh.addNormal(f.normal);
                _mesh.addNormal(f.normal);
                _mesh.addNormal(f.normal);
                _mesh.addNormal(f.normal);
            }
        }

        _redrawMesh = false;
    }
    _mesh.draw();
}

void Quad::drawWireframe()
{
    for (auto &f: _faces) {
        auto v0 = _vertices[_edges[f.edges[0]].vertex];
        auto v1 = _vertices[_edges[f.edges[1]].vertex];
        auto v2 = _vertices[_edges[f.edges[2]].vertex];
        auto v3 = _vertices[_edges[f.edges[3]].vertex];
        ofLine(v0.position, v1.position);
        ofLine(v1.position, v2.position);
        ofLine(v2.position, v3.position);
        ofLine(v3.position, v0.position);
    }
}

EdgeID Quad::findEdge(VertexID v0, VertexID v1)
{
    auto key = makeEdgeKey(v0, v1);
    auto result = _edgeMap.find(key);
    if (result == _edgeMap.end()) {
        return -1;
    }
    return result->second;
}

// Catmull-Clark subdivision surface algorithm
Quad Quad::subdivide(int level)
{
    if (level == 0) {
        return *this;
    }

    for (auto &v: _vertices) {
        v.newVertex = -1;
    }
    
    vector<ofVec3f> newVertices;
        
    // Divide existing face into four new faces, using the four existing face
    // vertices and a new vertex at the center of existing face. Calculate new
    // vertex in center of existing face by averaging four corner vertices
    // of face.
    for (auto &f: _faces) {
        newVertices.push_back((_vertices[_edges[f.edges[0]].vertex].position +
                               _vertices[_edges[f.edges[1]].vertex].position +
                               _vertices[_edges[f.edges[2]].vertex].position +
                               _vertices[_edges[f.edges[3]].vertex].position) / 4.0f);
        f.center = newVertices.size() - 1;
    }

    // Divide existing edge into two new edges, using edge endpoints and new
    // vertex around the midpoint of edge. Calculate new vertex on existing
    // edge by averaging the endpoints of edge and the centers of the two
    // adjacent faces.
    auto newMidpoint = [this, &newVertices](EdgeID edge, VertexID a, VertexID b) {
        // Check to see if new midpoint has already been calculated for
        // opposite edge
        if (_edges[_edges[edge].opposite].midpoint == -1) {
            auto newVertex = (_vertices[a].position +
                              _vertices[b].position +
                              newVertices[_faces[_edges[edge].face].center] +
                              newVertices[_faces[_edges[_edges[edge].opposite].face].center]) / 4.0f;
            newVertices.push_back(newVertex);
            _edges[edge].midpoint = newVertices.size() - 1;
            _edges[_edges[edge].opposite].midpoint = newVertices.size() - 1;
        }
        else {
            // We've already calculated the new midpoint vertex for the
            // opposite edge; no need to calculate again.
            _edges[edge].midpoint = _edges[_edges[edge].opposite].midpoint;
        }
    };

    for (auto &f: _faces) {
        newMidpoint(f.edges[0], _edges[f.edges[0]].vertex, _edges[f.edges[1]].vertex);
        newMidpoint(f.edges[1], _edges[f.edges[1]].vertex, _edges[f.edges[2]].vertex);
        newMidpoint(f.edges[2], _edges[f.edges[2]].vertex, _edges[f.edges[3]].vertex);
        newMidpoint(f.edges[3], _edges[f.edges[3]].vertex, _edges[f.edges[0]].vertex);
    }

    // Calculate new position of each existing vertex, using the midpoints of
    // connected edges, centers of connected faces, and current position.
    // Valence value is the number of connected edges.
    auto getNewVertex = [this, &newVertices](EdgeID edge) {
        int valence = 0;
        ofVec3f  sumMidpoints = {0.0, 0.0, 0.0};
        ofVec3f sumCenters = {0.0, 0.0, 0.0};

        // Loop through each connected edge of vertex
        auto e = _edges[edge].opposite;
        do {
            sumMidpoints += (_vertices[_edges[e].vertex].position +
                             _vertices[_edges[_edges[e].opposite].vertex].position) / 2.0f;
            sumCenters += newVertices[_faces[_edges[e].face].center];
            valence++;
            e = _edges[_edges[e].next].opposite;
        } while (_edges[e].opposite != edge);

        // Formulate for calculating new vertex position
        auto newVertex = ((sumCenters / valence) +
                          ((sumMidpoints / valence) * 2) +
                          (_vertices[_edges[edge].vertex].position * (valence - 3))) / valence;

        newVertices.push_back(newVertex);
        return newVertices.size() - 1;
    };
    
    for (auto &f: _faces) {
        auto v0 = _edges[f.edges[0]].vertex;
        auto v1 = _edges[f.edges[1]].vertex;
        auto v2 = _edges[f.edges[2]].vertex;
        auto v3 = _edges[f.edges[3]].vertex;
        _vertices[v0].newVertex = getNewVertex(f.edges[0]);
        _vertices[v1].newVertex = getNewVertex(f.edges[1]);
        _vertices[v2].newVertex = getNewVertex(f.edges[2]);
        _vertices[v3].newVertex = getNewVertex(f.edges[3]);
    }
    
    // Build new quad
    Quad newQuad;
    for (auto &v: newVertices) {
        newQuad.addVertex(v);
    }

    for (auto &f: _faces) {
        auto e0 = _edges[f.edges[0]];
        auto e1 = _edges[f.edges[1]];
        auto e2 = _edges[f.edges[2]];
        auto e3 = _edges[f.edges[3]];

        auto v0 = _vertices[e0.vertex].newVertex;
        auto v1 = _vertices[e1.vertex].newVertex;
        auto v2 = _vertices[e2.vertex].newVertex;
        auto v3 = _vertices[e3.vertex].newVertex;

        auto mp0 = e0.midpoint;
        auto mp1 = e1.midpoint;
        auto mp2 = e2.midpoint;
        auto mp3 = e3.midpoint;

        auto center = f.center;

        newQuad.addFace(v0, mp0, center, mp3);
        newQuad.addFace(v1, mp1, center, mp0);
        newQuad.addFace(v2, mp2, center, mp1);
        newQuad.addFace(v3, mp3, center, mp2);
    }

    if (level > 1) {
        return newQuad.subdivide(level - 1);
    }
    return newQuad;
}

// Calculate smooth normals for each vertex by averaging normal of each face
// that vertex is a part of
void Quad::calculateNormals()
{
    // Calculate normal for each pair of edges in face, and average together
    // to get face normal. I ~think~ this should work for slightly coplanar
    // faces.
    for (auto &f: _faces) {
        auto v0 = &_vertices[_edges[f.edges[0]].vertex];
        auto v1 = &_vertices[_edges[f.edges[1]].vertex];
        auto v2 = &_vertices[_edges[f.edges[2]].vertex];
        auto v3 = &_vertices[_edges[f.edges[3]].vertex];
        auto n0 = ((v0->position - v1->position).getCrossed(v0->position - v3->position)).getNormalized();
        auto n1 = ((v2->position - v3->position).getCrossed(v2->position - v1->position)).getNormalized();
        f.normal = (n0 + n1) / 2.0;
    }
    
    // Calculate smoothed normal for each vertex
    for (auto &edge: _edges) {
        int numNormals = 0;
        ofVec3f sumNormals = {0.0, 0.0, 0.0};
        auto e = edge;
        do {
            sumNormals += _faces[e.face].normal;
            numNormals++;
            e = _edges[_edges[e.opposite].next];
        } while (e.face != edge.face);
        _vertices[edge.vertex].normal = sumNormals / numNormals;
    }
}
