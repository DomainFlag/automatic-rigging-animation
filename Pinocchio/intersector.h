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

#ifndef INTERSECTOR_H
#define INTERSECTOR_H

#include "mesh.h"
#include "vecutils.h"

class PINOCCHIO_API Intersector {
public:
    Intersector() : mesh(NULL) {}
    Intersector(const Mesh &m, const Pinocchio::Vector3 &inDir) : mesh(&m), dir(inDir) { init(); }
    
    vector<Pinocchio::Vector3> intersect(const Pinocchio::Vector3 &pt, vector<int> *outIndices = NULL) const;    
    const Pinocchio::Vector3 &getDir() const { return dir; }
private:
    void init();
    void getIndex(const Pinocchio::Vector2 &pt, int &x, int &y) const;
    
    const Mesh *mesh;
    Pinocchio::Vector3 dir;
    Pinocchio::Vector3 v1, v2; //basis
    Rect2 bounds; //within the basis
    
    vector<Pinocchio::Vector2> points;
    vector<Pinocchio::Vector3> sNormals; //they are scaled for intersection
    vector<vector<int> > triangles;
};

#endif //INTERSECTOR_H
