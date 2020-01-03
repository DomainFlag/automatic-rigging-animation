#include "Windows.h"

#include "src/Device.h"
#include "src/Renderer.h"

#include <string>
#include <cassert>

#define SKELETON_RENDER 1
#define SKELETON_CACHE 1

extern const string FILE_CACHE = "skeleton.out";

using namespace std;

int main() {
    assert (!SKELETON_RENDER && !SKELETON_CACHE);

    Device device;

    if (!device.initKinect()) {
        printf("Coudn't connect to the Kinect device...\n");
        exit(-1);
    } else {
        if (SKELETON_CACHE) {
            device.initCache(FILE_CACHE);
        }

        if (SKELETON_RENDER) {
            Renderer renderer(device);
            renderer.clear();
        } else {
            // Get skeleton data while available joints are generated
            for(int g = 0; g < device.frames; g++) {
                device.getSkeletalData();
            }
        }
    }

    device.clear();

    return 0;
}