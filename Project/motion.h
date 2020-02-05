#ifndef MOTION_H
#define MOTION_H

#include "Device.h"
#include "../Pinocchio/transform.h"
#include <unordered_map>

struct Rotation {
    Pinocchio::Vector4 rotation;

    int joint1;
    int joint2;
};

class Motion
{
public:
    Motion(Device * device, const string & filename);

    vector<vector<Pinocchio::Vector3>> positions;
    vector<vector<Rotation>> rotations;
    vector<unordered_map<int, Rotation>> hierarchicalRotations;

    bool empty() const { return data.empty(); }
    vector<Transform<> > get() const;
    vector<Pinocchio::Vector3> getPose() const;
    vector<Pinocchio::Vector3> getRefPose() const { return refPose; }
    vector<Pinocchio::Vector3> getSkeletonBones();
    double getLegLength() const { return legLength; }
    double getLegWidth() const { return legWidth; }
    vector<Rotation> getRelative();
    unordered_map<int, Rotation> getHierarchicalRelative();

    const vector<vector<Transform<> > > &getData() const { return data; }
    void setFixedFrame(int inFrame) { fixedFrame = inFrame < 0 ? -1 : (int)(inFrame % data.size()); }
    void setFrameIdx();
    void getDeviceData();

    int frameIdx = -1;
private:
    void readRawFile(istream & strm);
    vector<vector<Transform<>>> data;
    vector<vector<Pinocchio::Vector3>> poses;
    vector<Pinocchio::Vector3> refPose;
    Device* device = nullptr;

    double legLength;
    double legWidth;
    int fixedFrame;
};

#endif
