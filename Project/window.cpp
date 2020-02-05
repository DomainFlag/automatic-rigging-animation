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

#include <FL/gl.h>
#include <FL/Fl.H>
#include "window.h"
#include "../Pinocchio/skeleton.h"
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#define COLOR_SEED 121

static HumanSkeleton human;

MyWindow *win = NULL;

void idle(void *s)
{
    if(win != NULL)
        win->redraw();
}

MyWindow::MyWindow() : Fl_Gl_Window(1024, 768, "Pinocchio"), flatShading(false), floor(true), skeleton(false)
{
    size_range(20, 20, 5000, 5000);
    end();

    resetTransform();
    win = this;
    Fl::add_idle(idle);
}

static int prevX, prevY;

int MyWindow::handle(int event) {
    switch(event) {
    case FL_PUSH:
        prevX = Fl::event_x();
        prevY = Fl::event_y();
        return 1;
    case FL_DRAG:
        if(Fl::event_state(FL_BUTTON3)) {
            int dx = Fl::event_x() - prevX, dy = Fl::event_y() - prevY;
            double len = sqrt(double(SQR(dx) + SQR(dy))) * 0.01;
            Transform<> cur = Transform<>(Pinocchio::Vector3(0.5, 0.5, 0.5)) *
                Transform<>(Quaternion<>(Pinocchio::Vector3(dy, dx, 0), len)) *
                Transform<>(Pinocchio::Vector3(-0.5, -0.5, -0.5));

            transform = cur * transform;
        }
        else if(Fl::event_state(FL_BUTTON1)) {
            double scale = min(w(), h()) / 2.5;
            Transform<> cur = Transform<>(Pinocchio::Vector3(Fl::event_x() - prevX, prevY - Fl::event_y(), 0) / scale);

            transform = cur * transform;
        }
        prevX = Fl::event_x();
        prevY = Fl::event_y();

        return 1;
    case FL_MOUSEWHEEL:
        {
            double scale = exp(-double(Fl::event_dy()) / 10.);

            Transform<> cur = Transform<>(Pinocchio::Vector3(0.5, 0.5, 0.5)) *
                    Transform<>(scale) *
                    Transform<>(Pinocchio::Vector3(-0.5, -0.5, -0.5));
                
            transform = cur * transform;
        }
        return 1;

    case FL_KEYBOARD:
        switch(Fl::event_key()) {
        case 't':
            resetTransform();
            break;
        case 'p':
            toggle = !toggle;
            break;
        case 'z':
            transform = Transform<>();
            break;
        case 's':
            skeleton = !skeleton;
            break;
        case 'f':
            flatShading = !flatShading;
            break;
        case 'g':
            floor = !floor;
            break;
        default:
            break;
        }
        return 1;
    default:
        // pass other events to the base class...
        return Fl_Gl_Window::handle(event);
    }
}

void getColor() {
    int number = rand();

    glColor3d((float) (number % 255) / 255, (float) ((number + 200) % 255) / 255, (float) ((number + 521)  % 255) / 255);
}

void MyWindow::addMesh(DeformableMesh * mesh) {     
    meshes.push_back(mesh);
}

void MyWindow::loadFileTexture(string filename, int material) {
    int width, height, channels;
    unsigned char * image = stbi_load(filename.c_str(), &width, &height, &channels, 0);

    if(image == nullptr) {
        cout << "Failed to load texture: " + filename << endl;
        cout << stbi_failure_reason() << endl;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    if (texture == -1) {
        cout << "Failed to generate a texture name: " + filename << endl;
    }

    textures.insert({material, texture});

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    if(channels == 3)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    else if(channels == 4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(image);
}

void MyWindow::draw() {
    int i;
    if (!valid()) {
        initGL();

        double w = this->w(), h = this->h();
        glViewport(0, 0, (int)w, (int)h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        double left = -.1;
        double bottom = -.1;
        double right = 1.1;
        double top = 1.1;
        if(w > 1 && h > 1) {
            if(w > h) {
                right = -0.1 + 1.2 * w / h;
            }
            if(h > w) {
                bottom = 1.1 - 1.2 * h / w;
            }
        }

        double scale = 1. / 1000.;
        left = -w * scale;
        right = w * scale;
        bottom = -h * scale;
        top = h * scale;
        glFrustum(left, right, bottom, top, 5., 30.);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    //Transform------
    Pinocchio::Vector3 trans = transform.getTrans();
    glTranslated(trans[0], trans[1], -10 + trans[2]);
    
    double scale = transform.getScale();
    glScaled(scale, scale, scale);
    
    Quaternion<> r = transform.getRot();
    double ang = r.getAngle();
    if(fabs(ang) > 1e-6) {
        Pinocchio::Vector3 ax = r.getAxis();
        glRotated(ang * 180. / M_PI, ax[0], ax[1], ax[2]);
    }

    //Draw----------
    if(floor)
        drawFloor();

    vector<const Mesh *> ms(meshes.size());
    for(i = 0; i < (int)meshes.size(); ++i) {
        meshes[i]->motion->setFrameIdx();
        ms[i] = &(meshes[i]->getMesh());
    }

    //shadows
    // if(floor) {
    //     Vector3 lightRay = transform.getRot().inverse() * Vector3(1, 2, 2);
    //     if(lightRay[1] == 0)
    //         lightRay[1] = 1e-5;
    //     lightRay = -lightRay / lightRay[1];
            
    //     glDisable(GL_LIGHTING);
    //     glColor3f(0.1f, 0.1f, 0.1f);
    //     glPushMatrix();
    //     float matr[16] = {1,0,0,0, (float)lightRay[0],0,(float)lightRay[2],0, 0,0,1,0, 0,0.01f,0,1};
    //     glMultMatrixf(matr);
    //     glDepthMask(0);
    //     for(i = 0; i < (int)ms.size(); ++i)
    //         drawMesh(*(ms[i]), flatShading);                
    //     glDepthMask(1);
    //     glEnable(GL_LIGHTING);
    //     glPopMatrix();
    // }

    static GLfloat colr[4] = {1.f, .9f, .75f, 1.0f };
    static GLfloat colrb[4] = {1.f, .9f, .75f, 1.0f };
    glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colr);
    glMaterialfv( GL_BACK, GL_AMBIENT_AND_DIFFUSE, colrb);

    //draw meshes
    for(i = 0; i < (int)meshes.size(); ++i) {
        drawMesh(*(ms[i]), flatShading);
    }

    //draw lines
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    for(i = 0; i < (int)lines.size(); ++i) {
        glColor3d(lines[i].color[0], lines[i].color[1], lines[i].color[2]);
        glLineWidth((float)lines[i].thickness);
        glBegin(GL_LINES);
        glVertex3d(lines[i].p1[0], lines[i].p1[1], lines[i].p1[2]);
        glVertex3d(lines[i].p2[0], lines[i].p2[1], lines[i].p2[2]);
        glEnd();
    }

    if(skeleton) {
        glLineWidth(5);
        for(i = 0; i < (int)meshes.size(); ++i) {
            vector<Pinocchio::Vector3> avatar = meshes[i]->getSkeletonAvatar();
            if(avatar.size() == 0)
                continue;

            srand(COLOR_SEED);

            const vector<int> &prev = human.fPrev();

            glBegin(GL_LINES);
            for(int j = 1; j < (int) prev.size(); ++j) {
                int k = prev[j];
                getColor();
                glVertex3d(avatar[j][0], avatar[j][1], avatar[j][2]);
                glVertex3d(avatar[k][0], avatar[k][1], avatar[k][2]);
            }
            glEnd();

            srand(COLOR_SEED);
            vector<Pinocchio::Vector3> original = meshes[i]->getSkeletonTracked();
            
            glBegin(GL_LINES);
            for(int j = 1; j < (int) prev.size(); ++j) {
                int k = prev[j];
                getColor();
                glVertex3d(original[j][0], original[j][1], original[j][2]);
                glVertex3d(original[k][0], original[k][1], original[k][2]);
            }
            glEnd();
        }
    }
}

void MyWindow::resetTransform()
{
    transform = Transform<>(Quaternion<>(Pinocchio::Vector3(1, 0, 0), .2), 1.5, Pinocchio::Vector3(-1.66, -0.66, 0));
}

void MyWindow::initGL()
{
    static GLfloat pos[4] = {5.0, 5.0, 10.0, 1.0 };

    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
    glLightfv( GL_LIGHT0, GL_POSITION, pos );
    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_NORMALIZE);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_ALPHA_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_FLAT);
    glClearColor(0.f, 0.f, 0.f, 0.f);

    for(int i = 0; i < (int) meshes.size(); ++i) {
        for (auto iter: this->meshes[i]->origMesh.materials) {
            Material & material = iter.second;
            
            loadFileTexture(material.path, material.index);
        }

        cout << "Available Textures: " << textures.size() << endl;
    }
}

void MyWindow::drawMesh(const Mesh &m, bool flatShading, Pinocchio::Vector3 trans)
{
    int i = 0;
    Pinocchio::Vector3 normal;

    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);

    glPointSize(10.0f);
    glBegin(GL_POINTS);
    for (auto iter: m.joints) {
        Pinocchio::Vector3 & joint = iter.second;
        glVertex3f(joint[0], joint[1], joint[2]);
    }
    glEnd();

    glColor3f(0, 0, 1.0);
    if (toggle) {
        int tex = -1;
        while (i < m.edges.size()) {
            if (tex != -1) {
                glBindTexture(GL_TEXTURE_2D, tex);
            }

            glBegin(GL_TRIANGLES);

            for (; i < (int)m.edges.size(); i++) {
                int v = m.edges[i].vertex;
                const Pinocchio::Vector3& p = m.vertices[v].pos;
                const Pinocchio::Vector3& t = m.vertices[v].texture;

                auto iter = textures.find(t[2]);
                if (iter != textures.end()) {
                    int texCurr = iter->second;
                    if (tex != texCurr) {
                        tex = texCurr;

                        if (tex != -1) {
                            break;
                        }
                    }
                }
                else {
                    tex = -1;
                }

                if (!flatShading) {
                    normal = m.vertices[v].normal;
                    glNormal3d(normal[0], normal[1], normal[2]);
                } else if (i % 3 == 0) {
                    const Pinocchio::Vector3& p2 = m.vertices[m.edges[i + 1].vertex].pos;
                    const Pinocchio::Vector3& p3 = m.vertices[m.edges[i + 2].vertex].pos;

                    normal = ((p2 - p) % (p3 - p)).normalize();
                    glNormal3d(normal[0], normal[1], normal[2]);
                }

                if (tex != -1 && t[2] != -1) {
                    glTexCoord2f(t[0], t[1]);
                }

                glVertex3d(p[0], p[1], p[2]);
            }

            glEnd();
        }
    }
}

void MyWindow::drawFloor()
{
    int i;
    Mesh floor;
    floor.vertices.resize(4);
    for(i = 0; i < 4; ++i) {
        floor.vertices[i].normal = Pinocchio::Vector3(0, 1, 0);
        floor.vertices[i].pos = 10. * Pinocchio::Vector3(((i + 0) % 4) / 2, 0, ((i + 1) % 4) / 2) - Pinocchio::Vector3(4.5, 0, 4.5);
    }
    floor.edges.resize(6);
    for(i = 0; i < 6; ++i)
        floor.edges[i].vertex = (i % 3) + ((i > 3) ? 1 : 0);

    static GLfloat colrb[4] = {0.5f, .9f, .75f, 1.0f };
    static GLfloat colr[4] = {0.5f, .6f, .9f, 1.0f };
    glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colr);
    glMaterialfv( GL_BACK, GL_AMBIENT_AND_DIFFUSE, colrb);

    glShadeModel(GL_SMOOTH);
    drawMesh(floor, false);
    glShadeModel( flatShading ? GL_FLAT : GL_SMOOTH);

    glColor4d(.5, .6, .9, .3);
    glLineWidth(1.);
    int gridSize = 20;
    double y = floor.vertices[0].pos[1];
    double minX = floor.vertices[1].pos[0];
    double maxX = floor.vertices[2].pos[0];
    double minZ = floor.vertices[1].pos[2];
    double maxZ = floor.vertices[3].pos[2];
    double stepX = (maxX - minX) / double(gridSize);
    double stepZ = (maxZ - minZ) / double(gridSize);
    glEnable(GL_BLEND);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glBegin(GL_LINES);
    for(i = 0; i <= gridSize; ++i) {
        glVertex3d(minX + i * stepX, y, minZ);
        glVertex3d(minX + i * stepX, y, maxZ);
        glVertex3d(minX, y, minZ + i * stepZ);
        glVertex3d(maxX, y, minZ + i * stepZ);
    }
    glEnd();
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}
