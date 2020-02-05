#include <FL/Fl.H>
#include "window.h"
#include "motion.h"
#include "defmesh.h"
#include "device.h"
#include "../Pinocchio/skeleton.h"
#include "../Pinocchio/pinocchioApi.h"

#define SKELETON_RENDER 1
#define SKELETON_CACHE 0

extern const string FILE_CACHE = "skeleton.out";

int main(int argc, char** argv) {
    if (argc != 6) {
        cout << "Missing either {meshPath} {skeletonPath} {motionPath} {rigged} {animation}" << endl;
        exit(-1);
    }

    const string meshPath = argv[1]; 
    const string skeletonPath = argv[2];
    const string motionPath = argv[3];
    const string rigged = argv[4];
    const string animation = argv[5];

    bool SKELETON_RIGGED = false;
    if (rigged == "rigged") {
        SKELETON_RIGGED = true;
    } else {
        SKELETON_RIGGED = false;
    }

    bool SKELETON_ANIMATION = false;
    if (animation == "live") {
        SKELETON_ANIMATION = true;
    } else {
        SKELETON_ANIMATION = false;
    }

    Debugging::setOutStream(cout);

    MyWindow * window = new MyWindow();
    Mesh mesh(meshPath);

    Skeleton skelton = FileSkeleton(skeletonPath);
    if (!SKELETON_RIGGED) {
        skelton.scale(0.7);

        mesh.normalizeBoundingBox();
    }

    Device * device = nullptr;
    if (SKELETON_ANIMATION) {
        device = new Device();
    }

    DeformableMesh * defmesh;
    Motion * motion = new Motion(device, motionPath);
    if (SKELETON_RIGGED) {
        defmesh = new DeformableMesh(mesh, skelton, motion);
    } else {
        // compute joint skin association and weights to bones attachment
        PinocchioOutput riggedOut = autorig(skelton, mesh);
        defmesh = new DeformableMesh(mesh, skelton, riggedOut.embedding, *(riggedOut.attachment), motion);
    }

    defmesh->rigged = SKELETON_RIGGED;

    window->addMesh(defmesh);
    window->show();

    return Fl::run();
}