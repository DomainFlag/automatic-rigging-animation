/*
Copyright (c) 2007 Ilya Baran

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef MYWINDOW_H
#define MYWINDOW_H

#include <FL/Fl_Gl_Window.H>
#include "../Pinocchio/mesh.h"
#include "../Pinocchio/transform.h"
#include "defmesh.h"
#include <unordered_map>

struct LineSegment
{
    LineSegment() : thickness(1.) {}
    LineSegment(const Pinocchio::Vector3 &inP1, const Pinocchio::Vector3 &inP2,
        const Pinocchio::Vector3 &inColor = Pinocchio::Vector3(1, 1, 1), double inThickness = 1.)
        : p1(inP1), p2(inP2), color(inColor), thickness(inThickness) {}

    Pinocchio::Vector3 p1, p2, color;
    double thickness;
};

class MyWindow : public Fl_Gl_Window
{
public:
    MyWindow();

    virtual ~MyWindow() {}
    virtual void draw();
    virtual int handle(int);

    void addMesh(DeformableMesh * mesh);
    void addLine(const LineSegment &l) { lines.push_back(l); }
    void clearLines() { lines.clear(); }

private:
    bool flatShading, floor, skeleton, toggle = true;
    Transform<> transform;
    vector<DeformableMesh *> meshes;
    vector<LineSegment> lines;
    unordered_map<int, int> textures;

    void resetTransform();
    void initGL();
    void drawMesh(const Mesh &m, bool flatShading, Pinocchio::Vector3 trans = Pinocchio::Vector3());
    void drawFloor();
    void loadFileTexture(string filename, int material);
};

#endif //MYWINDOW_H
