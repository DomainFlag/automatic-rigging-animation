#include "motion.h"
#include "defmesh.h"
#include "../Pinocchio/skeleton.h"
#include "../Pinocchio/utils.h"

#include <fstream>
#include <sstream>

using namespace std;

Motion::Motion(Device * device, const string &file) : device(device), fixedFrame(-1) {
    if (device) {
        if (!device->initKinect()) {
            printf("Couldn't connect to the Kinect device...\n");
            exit(-1);
        } else {
            printf("Successfully connected to the Kinect device...\n");
            this->fixedFrame = 0;
        }
    } else {
        ifstream strm(file.c_str());

        if (!strm.is_open()) {
            cout << "Error opening file " << file << endl;
            return;
        }

        cout << "Reading " << file << endl;

        readRawFile(strm);
    }
}

void Motion::getDeviceData() {
    // Get skeleton data while available joints are generated
    bool flag = false;
    while (!flag) {
        flag = this->device->getSkeletalData();
    }

    const int size = 20;
    vector<Pinocchio::Vector3> positions;
    vector<Rotation> rotations;

    for (unsigned int g = 0; g < size; g++) {
        const Vector4& joint = device->skeletonPositions[g];

        positions.push_back(Pinocchio::Vector3(joint.x, joint.y, joint.z));
    }

    for (unsigned int g = 0; g < size; g++) {
        const _NUI_SKELETON_BONE_ORIENTATION& orientation = device->orientations[g];

        Rotation rotation;

        rotation.joint1 = orientation.startJoint;
        rotation.joint2 = orientation.endJoint;

        const Vector4& quart = orientation.absoluteRotation.rotationQuaternion;
        rotation.rotation = Pinocchio::Vector4(quart.x, quart.y, quart.z, quart.w);

        rotations.push_back(rotation);
    }

    this->positions = vector<vector<Pinocchio::Vector3>> { positions };
    this->rotations = vector<vector<Rotation>> { rotations };
}

void Motion::readRawFile(istream & stream) {
    const int size = 20;
    while(!stream.eof()) {
        vector<Pinocchio::Vector3> positions(size);
        vector<Rotation> rotations(size);
        unordered_map<int, Rotation> hierarchicalRotations;

        for(int g = 0; g < size * 3; g++) {
            stream >> positions[g / 3][g % 3];
        }

        for(int g = 0; g < size; g++) {
            Rotation & rotation = rotations[g];
            stream >> rotation.joint1 >> rotation.joint2;

            for(int h = 0; h < 4; h++) {
                stream >> rotation.rotation[h];
            }

            hierarchicalRotations.insert({rotation.joint2, rotation});
        }

        this->positions.push_back(positions);
        this->rotations.push_back(rotations);
        this->hierarchicalRotations.push_back(hierarchicalRotations);
    }

    // translate and scale relative to avatar skeleton
    Pinocchio::Vector3 translation = Pinocchio::Vector3(0.492356, 0.762127, 0.476605) - positions[0][0];
    double scale = 0.235319 / (positions[0][2] - positions[0][0]).length();
    Transform<> trans(scale, translation);

    for(int g = 0; g < this->positions.size(); g++) {
        for(int h = 0; h < size; h++) {
            this->positions[g][h] = trans * this->positions[g][h];
        }
    }
}

#ifdef _WIN32
#include "windows.h"

long getT()
{
    SYSTEMTIME systime;
    GetSystemTime(&systime);

    return systime.wMilliseconds + 1000 * (systime.wSecond + 60 * (systime.wMinute + 60 * (systime.wHour + 24 * systime.wDay)));
}
#else
#include <sys/time.h>

long getT()
{
    struct timeval tv;
    gettimeofday (&tv, NULL);

    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
#endif

int getMsecs()
{
    static unsigned long startTime = getT();
    return getT() - startTime;
}

void Motion::setFrameIdx() {
    if (fixedFrame >= 0) {
        this->getDeviceData();
        this->frameIdx = fixedFrame;
    }

    this->frameIdx = (getMsecs() / (1000 / 15)) % positions.size();
}

vector<Transform<>> Motion::get() const
{
    return data[this->frameIdx];
}

vector<Rotation> Motion::getRelative()
{
    return rotations[this->frameIdx];
}

vector<Pinocchio::Vector3> Motion::getSkeletonBones()
{
    return positions[this->frameIdx];
}

unordered_map<int, Rotation> Motion::getHierarchicalRelative() {
    return hierarchicalRotations[this->frameIdx];
}

vector<Pinocchio::Vector3> Motion::getPose() const
{
    return poses[this->frameIdx];
}

