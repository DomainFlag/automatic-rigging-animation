#ifndef RENDERER_H
#define RENDERER_H

#include "Device.h"
#include "Program.h"
#include "Tools.h"

class Renderer: public Program {
public:
    Device & device;

    GLint size;
    GLfloat * buffer;

    Renderer(Device & device): device(device) {
        initRenderer("../shaders/skeleton");
        setClearColor(0, 0, 0, 1.0);

        glEnable(GL_PROGRAM_POINT_SIZE);

        size = device.getSize();
        buffer = new GLfloat[size * 3];

        initUniform(perspective(M_PI / 3, WIN_SIZE_W / WIN_SIZE_H, 0.5f, 100.0f).data(), "u_camera");
        initBuffer(buffer, "a_position", size, 3);
    }

    void update() override {
        if(device.getSkeletalData()) {
            device.copy(buffer);

            updateBuffer(buffer, "a_position");
        } else {
            clear();
        }
    }

    void render() override {
        glDrawArrays(GL_POINTS, 0, size);
    }

    void clear() override {
        Program::clear();

        delete [] buffer;
    }
};

#endif