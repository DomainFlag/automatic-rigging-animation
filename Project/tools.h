#ifndef SKELETON_TOOLS_H
#define SKELETON_TOOLS_H

#define _USE_MATH_DEFINES

#include <Eigen/Dense>
#include <math.h>

Eigen::Matrix4f perspective(float fieldOfView, float aspect, float v_near, float v_far) {
    float f = tan(M_PI * 0.5 - 0.5 * fieldOfView);
    float rangeInv = (float) 1.0 / (v_near - v_far);

    Eigen::Matrix4f perspective;
    perspective << f / aspect, 0, 0, 0,
        0, f, 0, 0,
        0, 0, rangeInv * (v_near + v_far), -1,
        0, 0, rangeInv * v_near * v_far * 2, 0;

    return perspective;
}

#endif
