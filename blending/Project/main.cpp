#include <FL/Fl.H>
#include "MyWindow.h"
#include "motion.h"
#include "defmesh.h"
#include "../Pinocchio/skeleton.h"
#include "../Pinocchio/pinocchioApi.h"

int main(int argc, char **argv) {
    const string meshPath = argv[1];
    const string skeletonPath = argv[2];
    const string motionPath = argv[3];

    MyWindow *window = new MyWindow();
    Mesh mesh(meshPath);
    
    Skeleton skelton = FileSkeleton(skeletonPath);
    skelton.scale(0.7);

    mesh.normalizeBoundingBox();
    mesh.computeVertexNormals();

    // compute joint skin association and weights to bones attachment
    PinocchioOutput riggedOut = autorig(skelton, mesh);
    DeformableMesh defmesh(mesh, skelton, riggedOut.embedding, *(riggedOut.attachment), new Motion(motionPath));

    window->addMesh(& defmesh);
    window->show();

    return Fl::run();
}
