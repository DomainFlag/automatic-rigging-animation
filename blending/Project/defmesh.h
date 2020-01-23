#ifndef DEFMESH_H_INCLUDED
#define DEFMESH_H_INCLUDED

#include "../Pinocchio/attachment.h"
#include "motion.h"
#include <map>
#include <eigen3/Eigen/Dense>

struct JointTransformation {
    Eigen::Quaternion<double> rot;
    Eigen::Vector3d parent;
    Eigen::Vector3d childOld;
    Eigen::Vector3d childNew;
};


class DeformableMesh
{
public:
    DeformableMesh(const Mesh inMesh, const Skeleton &inOrigSkel, const vector<Vector3> &inMatch,
            const Attachment &inAttachment, Motion *inMotion = NULL) 
      : origSkel(inOrigSkel), match(inMatch), attachment(inAttachment), origMesh(inMesh), motion(inMotion) {

    }
      
    const int size() const {
        return motion->positions.size();
    };

    void computeBasePose();

    vector<Vector3> getSkel() const;
    vector<Vector3> getAvatarSkeleton();
    vector<Eigen::Quaternionf> getBasePose() const { return pose; };
    const Skeleton &getOrigSkel() const { return origSkel; }
    const Attachment &getAttachment() const { return attachment; }
    const Mesh & getMesh() { 
        updateMesh();

        return curMesh; 
    }

    mutable vector<Vector3> match;
    vector<Eigen::Quaternionf> pose;
private:
    map<int, Eigen::Transform<float, 3, Eigen::Affine>> computeTransforms() const;
    void updateMesh() const;

    Skeleton origSkel;
    Attachment attachment;
    mutable Mesh origMesh;
    mutable Mesh curMesh;
    Motion *motion;
};

#endif //DEFMESH_H_INCLUDED
