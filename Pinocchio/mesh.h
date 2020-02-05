/*  This file is part of the Pinocchio automatic rigging library.
    Copyright (C) 2007 Ilya Baran (ibaran@mit.edu)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>
#include <unordered_map>

#include "vector.h"
#include "rect.h"

using namespace std;

struct MeshVertex {
    MeshVertex() : edge(-1) {
        this->texture = Pinocchio::Vector3(0, 0, -1);
    }
    
    Pinocchio::Vector3 pos;
    Pinocchio::Vector3 normal;
    Pinocchio::Vector3 texture;
    unordered_map<string, double> weights;
    int edge; //an edge such that edge->prev->vertex is this
};

struct MeshEdge
{
    MeshEdge() : vertex(-1), prev(-1), twin(-1) {}
    
    int vertex; //the vertex the edge points to--the start vertex is prev->vertex
    int prev; //ccw, next is prev->prev
    int twin;
};

struct Material {
    Material(string path, int index): path(path), index(index) {};

    string path;
    int index;
};

class PINOCCHIO_API Mesh {
public:
    Mesh() : scale(1.) {}
    Mesh(const string &file);

    bool integrityCheck() const;
    bool isConnected() const; //returns true if the mesh consists of a single connected component
    void computeVertexNormals();
    void computeVertexTextures();
    void normalizeBoundingBox();
    void computeTopology();
    void writeObj(const string &filename) const;
    
private:
    void readObj(istream &strm);
    void readOff(istream &strm);
    void readPly(istream &strm);
    void readGts(istream &strm);
    void readStl(istream &strm);
    void readFbx(const string &file, const string &hint = "");
    void fixDupFaces();
    void sortEdges(); //sort edges so that triplets forming faces are adjacent

public: //data
    vector<MeshVertex> vertices;
    vector<MeshEdge> edges;
    vector<MeshVertex> textures;
    unordered_map<string, Pinocchio::Vector3> joints;
    unordered_map<string, Material> materials;
    Pinocchio::Vector3 toAdd;
    double scale;
};

#endif
