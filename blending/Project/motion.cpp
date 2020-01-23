#include "motion.h"
#include "defmesh.h"
#include "../Pinocchio/skeleton.h"
#include "../Pinocchio/utils.h"

#include <fstream>
#include <sstream>

using namespace std;

Motion::Motion(const string &file) : fixedFrame(-1) {
    ifstream strm(file.c_str());
    
    if(!strm.is_open()) {
        cout << "Error opening file " << file << endl;
        return;
    }
    
    cout << "Reading " << file << endl;
           
    readRawFile(strm);
}

void Motion::readRawFile(istream & stream) {
    const int size = 20;
    while(!stream.eof()) {
        vector<Vector3> positions(size);
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
    Vector3 translation = Vector3(0.492356, 0.762127, 0.476605) - positions[0][0];
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

int Motion::getFrameIdx() const
{
    if(fixedFrame >= 0)
        return fixedFrame;
    return (getMsecs() / (1000 / 15)) % positions.size();
}

vector<Transform<>> Motion::get() const
{
    return data[getFrameIdx()];
}

vector<Rotation> Motion::getRelative()
{
    return rotations[getFrameIdx()];
}

vector<Vector3> Motion::getSkeletonBones()
{
    return positions[getFrameIdx()];
}

unordered_map<int, Rotation> Motion::getHierarchicalRelative() {
    return hierarchicalRotations[getFrameIdx()];
}

vector<Vector3> Motion::getPose() const
{
    return poses[getFrameIdx()];
}

