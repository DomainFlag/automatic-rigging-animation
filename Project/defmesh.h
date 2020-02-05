#ifndef DEFMESH_H_INCLUDED
#define DEFMESH_H_INCLUDED

#include "../Pinocchio/attachment.h"
#include "motion.h"
#include <map>
#include <Eigen/Dense>

struct JointTransformation {
    Eigen::Quaternion<double> rot;
    Eigen::Vector3d parent;
    Eigen::Vector3d childOld;
    Eigen::Vector3d childNew;
};

class DeformableMesh
{
public:
    DeformableMesh() {};
    DeformableMesh(const Mesh inMesh, const Skeleton& inOrigSkel, const vector<Pinocchio::Vector3>& inMatch,
        const Attachment& inAttachment, Motion* inMotion = NULL)
        : origSkel(inOrigSkel), match(inMatch), attachment(inAttachment), origMesh(inMesh), motion(inMotion) {};

    DeformableMesh(const Mesh inMesh, const Skeleton &inOrigSkel, Motion *inMotion = NULL): origSkel(inOrigSkel), origMesh(inMesh), motion(inMotion) {
        for (int g = 0; g < inOrigSkel.joints.size(); g++) {
            string name = inOrigSkel.joints[g];
            auto iter = origMesh.joints.find(name);
            if (iter != origMesh.joints.end()) {
                match.push_back(iter->second);
                avatar.push_back(iter->second);
            } else {
                cout << name << " not existent" << endl;
            }
        }
    }
      
    const int size() const {
        return motion->positions.size();
    };

    void computeBasePose();

    vector<Pinocchio::Vector3> getSkeletonAvatar() const;
    vector<Pinocchio::Vector3> getSkeletonTracked();
    vector<Eigen::Quaternionf> getBasePose() const { return pose; };
    const Skeleton &getOrigSkel() const { return origSkel; }
    const Attachment &getAttachment() const { return attachment; }
    const Mesh & getMesh() { 
        updateMesh();

        return curMesh; 
    }
    mutable vector<Pinocchio::Vector3> match;
    mutable vector<Pinocchio::Vector3> avatar;
    vector<Eigen::Quaternionf> pose;
    mutable Mesh origMesh;
    bool rigged = false;
    Motion* motion;
private:
    map<int, Eigen::Transform<float, 3, Eigen::Affine>> computeTransforms() const;
    void updateMesh() const;

    Skeleton origSkel;
    Attachment attachment;
    mutable Mesh curMesh;
};

#endif //DEFMESH_H_INCLUDED
