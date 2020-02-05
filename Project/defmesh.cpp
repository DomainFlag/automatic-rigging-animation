#include "defmesh.h"
#include "../Pinocchio/intersector.h"


Eigen::Vector3f transformToEigen(const Pinocchio::Vector3 & vec) {
    return Eigen::Vector3f(vec[0], vec[1], vec[2]);
} 

Pinocchio::Vector3 transformToVec(const Eigen::Vector3f & vec) {
    return Pinocchio::Vector3(vec.x(), vec.y(), vec.z());
}

Eigen::Vector3f getDirection(const Rotation & rot) {
    Eigen::Quaternionf quart(-rot.rotation[3], rot.rotation[0], rot.rotation[1], -rot.rotation[2]);

    return quart * Eigen::Vector3f(0, 1, 0);
}

Eigen::Quaternionf getRootRotation(const vector<Pinocchio::Vector3> & pose, const Rotation & rot) {
    Eigen::Vector3f directionRoot = getDirection(rot);
    Eigen::Quaternionf rootRotation = Eigen::Quaternionf::FromTwoVectors(Eigen::Vector3f(0, 1, 0), directionRoot);

    Eigen::Vector3f parent = transformToEigen(pose[rot.joint1]);
    Eigen::Vector3f child = transformToEigen(pose[rot.joint2]);

    Eigen::Vector3f directionPose = child - parent;
    Eigen::Vector3f directionAvatar = getDirection(rot);

    return Eigen::Quaternionf::FromTwoVectors(directionPose, directionAvatar);
}

map<int, Eigen::Transform<float, 3, Eigen::Affine>> DeformableMesh::computeTransforms() const {
    map<int, Eigen::Transform<float, 3, Eigen::Affine>> transforms;

    const vector<Rotation> rotations = motion->getRelative();

    vector<Pinocchio::Vector3> pose = match;
    Eigen::Quaternionf rootRotation = getRootRotation(match, rotations[0]);
    Eigen::Vector3f root = transformToEigen(pose[0]);

    for(int g = 1; g < rotations.size(); g++) {
        const Rotation & rot = rotations[g];

        Eigen::Vector3f parent = transformToEigen(pose[rot.joint1]);
        Eigen::Vector3f child = transformToEigen(pose[rot.joint2]);

        auto stack = transforms.find(rot.joint1);
        if (stack != transforms.end()) {
            parent = stack->second * parent;
            child = stack->second * child;
        }

        Eigen::Vector3f directionPose = child - parent;

        Eigen::Translation<float, 3> translForward(parent);
        Eigen::Translation<float, 3> translBack = translForward.inverse();

        Eigen::Vector3f directionAvatar = getDirection(rot);

        auto rotation = Eigen::Quaternionf::FromTwoVectors(directionPose, directionAvatar);
        Eigen::Transform<float, 3, Eigen::Affine> transform = translForward * rotation * translBack;
        if (transforms.find(rot.joint1) != transforms.end()) {
            transform = transform * transforms.find(rot.joint1)->second;
        } else {
            transform = transform * rootRotation;
        }

        transforms.insert({rot.joint2, transform});
    }

    return transforms;
}

void DeformableMesh::computeBasePose() {}

void DeformableMesh::updateMesh() const {
    map<int, Eigen::Transform<float, 3, Eigen::Affine>> transforms = computeTransforms();
    const vector<Pinocchio::Vector3> & positions = motion->getSkeletonBones();

    Eigen::Translation<float, 3> rootTranslation(transformToEigen(positions[0]).array() * Eigen::Vector3f(1.0, 1.0, -3.0).array());
    auto rotAxis = Eigen::AngleAxisf(M_PI, Eigen::Vector3f::UnitY());

    Eigen::Vector3f root = transformToEigen(match[0]);

    curMesh = origMesh;
    for(int i = 0; i < origMesh.vertices.size(); i++) {
        Eigen::Vector3f updatedPos(0, 0, 0);

        if (!rigged) {
            Vector<double, -1> weights = attachment.getWeights(i);
            for (int j = 0; j < weights.size(); j++) {
                updatedPos += weights[j] * (transforms.find(j + 1)->second * transformToEigen(origMesh.vertices[i].pos));
            }
        } else {
            unordered_map<string, double> weights = origMesh.vertices[i].weights;
            for (auto weight: weights) {
                int index = origSkel.joints_names.find(weight.first)->second;

                updatedPos += weight.second * (transforms.find(index)->second * transformToEigen(origMesh.vertices[i].pos));
            }
        }
        
        curMesh.vertices[i].pos = transformToVec(rootTranslation * updatedPos);
    }
    curMesh.computeVertexNormals();

    avatar = match;
    for (int g = 1; g < avatar.size(); g++) {
        avatar[g] = transformToVec(rootTranslation * transforms.find(g)->second * transformToEigen(avatar[g]));
    }
    avatar[0] = transformToVec(rootTranslation * transformToEigen(avatar[0]));
}

vector<Pinocchio::Vector3> DeformableMesh::getSkeletonTracked() {
    return motion->positions[motion->frameIdx];
}

vector<Pinocchio::Vector3> DeformableMesh::getSkeletonAvatar() const {
    return avatar;
}

