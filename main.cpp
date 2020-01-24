#include "Windows.h"

#include "src/Device.h"
#include "src/Renderer.h"
#include "src/Animation.h"

#include <string>
#include <cassert>

#define SKELETON_RENDER 1
#define SKELETON_CACHE 1
#define SKELETON_ANIMATION 0

extern const string FILE_CACHE = "skeleton.out";

using namespace std;

int main() {
    assert (SKELETON_RENDER || SKELETON_CACHE);

    if (SKELETON_ANIMATION) {
        Animation animation;
    } else {
        Device device;
        if (!device.initKinect()) {
            printf("Couldn't connect to the Kinect device...\n");
            exit(-1);
        } else {
            printf("Successfully connected to the Kinect device...\n");

            if (SKELETON_CACHE) {
                device.initCache(FILE_CACHE);
            }

            if (SKELETON_RENDER) {
                Renderer renderer(device);
                renderer.startRenderer();
                renderer.clear();
            } else {
                // Get skeleton data while available joints are generated
                bool flag = true;
                while(flag) {
                    flag = device.getSkeletalData();
                }
            }
        }

        device.clear();
    }

    return 0;
}
