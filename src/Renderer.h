#ifndef RENDERER_H
#define RENDERER_H

#include "Device.h"
#include "Program.h"

class Renderer: private Program {
public:
    Device & device;

    GLint size;
    GLfloat * buffer;

    Renderer(Device & device): device(device) {
        setClearColor(0, 0, 0, 1.0);

        initRenderer("shaders/skeleton");

        glEnable(GL_PROGRAM_POINT_SIZE);

        size = device.getSize();
        buffer = new GLfloat[size * 3];

        initBuffer(buffer, "a_position", size, 3);
    }

    void update() override {
        device.copy(buffer);

        updateBuffer(buffer, "a_position");
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