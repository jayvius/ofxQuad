#ifndef OFXQUAD_H
#define OFXQUAD_H

#include "ofMain.h"
#include <vector>
#include <array>
#include <unordered_map>
#include <utility>
#include <cstring>
#include <cstdint>
#include <string>


namespace ofx
{

typedef int32_t VertexID;
typedef int32_t EdgeID;
typedef int32_t FaceID;

typedef std::pair<VertexID, VertexID> EdgeKey;


// Functor for hashing edges. An edge is defined as two VertexIDs that
// represent the start and end points of edge. Removing vertices from
// vertex vector invalidates this hash value.
class EdgeHash
{
public:
    std::size_t operator()(const EdgeKey &key) const
    {
        auto v0 = key.first;
        auto v1 = key.second;
        // TODO: raise exception on invalid (negative) vertex indices
        if (v0 > v1) {
            auto temp = v0;
            v0 = v1;
            v1 = temp;
        }
        int64_t hashValue = v0;
        // TODO: guard against case where vertex index exceeds 32 bits
        return (hashValue << 32) | (v1 & 0xffffffff);
    }
};

struct Vertex
{
    ofVec3f position;

    // normal used for smooth shading
    ofVec3f normal;

    // ID representing new vertex position
    // when calculating subdivision surface
    VertexID newVertex;
};

struct Edge
{
    // ID representing start vertex
    VertexID vertex;

    // ID representing next connected edge in face
    EdgeID next;

    // ID representing mirror edge of adjacent face,
    // moving in the opposite direction
    EdgeID opposite;

    // ID representing face that this edge is a part of
    FaceID face;

    // ID representing midpoint of edge;
    // used when calculating subdivision surface
    VertexID midpoint;
};

struct Face
{
    std::array<EdgeID, 4> edges;

    // ID representing average of all vertices of face;
    // mainly used for calculating subdivision surface
    VertexID center;

    // normal used for flat shading
    ofVec3f normal;
};


// Quad polygon mesh. Mesh data is stored in half-edge data structure,
// where each face has it's own set of edges. Each actual edge of
// mesh is represented by two "half-edges", one for each adjacent face.
class Quad
{
public:
    Quad();

    // Create quad from OBJ file
    Quad(std::string objFilename);
    
    // Load mesh data from OBJ file
    void load(std::string objFilename);

    // Add vertex and return ID of new vertex
    VertexID addVertex(ofVec3f vertex);

    // Add vertexIDs for new face. Adjacent edges should share vertices.
    FaceID addFace(VertexID v0, VertexID v1, VertexID v2, VertexID v3);

    // Catmull-Clark subdivision surface
    Quad subdivide(int level=1);

    void draw(bool smoothShading=true);
    void drawWireframe();

private:
    std::vector<Vertex> _vertices;
    std::vector<Edge> _edges;
    std::vector<Face> _faces;
    std::unordered_map<EdgeKey, EdgeID, EdgeHash> _edgeMap;

    EdgeID addEdge(VertexID v0, VertexID v1, FaceID f0, FaceID f1);
    EdgeID findEdge(VertexID v0, VertexID v1);

    ofVboMesh _mesh;
    bool _redrawMesh;

    void calculateNormals();
};

};


#endif
