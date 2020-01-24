#ifndef DEVICE_H
#define DEVICE_H

#include <NuiApi.h>
#include <NuiSensor.h>

#include <thread>
#include <mutex>
#include <condition_variable>

#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <unordered_map>

using namespace std;

class Device {
public:

    // The kinect sensor
    INuiSensor * sensor = nullptr;

    // Initialize Skeleton Data
    Vector4 skeletonPositions[NUI_SKELETON_POSITION_COUNT];
    NUI_SKELETON_BONE_ORIENTATION orientations[NUI_SKELETON_POSITION_COUNT];

    int index = 0;
    bool consumed = true;
    bool cache = false;

    thread th;
    mutex mtx;
    condition_variable cv;

    vector<string> joints = {
            "HIP_CENTER",
            "SPINE",
            "SHOULDER_CENTER",
            "HEAD",
            "SHOULDER_LEFT",
            "ELBOW_LEFT",
            "WRIST_LEFT",
            "HAND_LEFT",
            "SHOULDER_RIGHT",
            "ELBOW_RIGHT",
            "WRIST_RIGHT",
            "HAND_RIGHT",
            "HIP_LEFT",
            "KNEE_LEFT",
            "ANKLE_LEFT",
            "FOOT_LEFT",
            "HIP_RIGHT",
            "KNEE_RIGHT",
            "ANKLE_RIGHT",
            "FOOT_RIGHT"
    };

    vector<int> mapping = {
            2, 1, 0, 3, 12, 13, 14, 15, 16, 17, 18, 19, 4, 5, 6, 8, 9, 10
    };

    unordered_map<int, int> mappingOrientations;

    int frames, delay;

    Device(int frames = 400, int delay = 200): frames(frames), delay(delay) {
        for (int g = 0; g < mapping.size(); g++) {
            mappingOrientations.insert({mapping[g], g});
        }
    };

    ~Device() {
        if(th.joinable()) {
            th.join();
        }
    }

    bool initKinect() {
        // Get a working kinect sensor
        int numSensors;
        if (NuiGetSensorCount(&numSensors) < 0 || numSensors < 1) return false;
        if (NuiCreateSensorByIndex(0, &sensor) < 0) return false;

        // Initialize sensor
        sensor->NuiInitialize(
                NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX
                | NUI_INITIALIZE_FLAG_USES_COLOR
                | NUI_INITIALIZE_FLAG_USES_SKELETON);

        // NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT for only upper body
        sensor->NuiSkeletonTrackingEnable(nullptr, 0);

        return sensor;
    }

    unsigned int getSize() {
        return NUI_SKELETON_POSITION_COUNT;
    }

    bool getSkeletalData() {
        if (index != -1 && index >= frames)
            return false;

        while(true) {
            NUI_SKELETON_FRAME skeletonFrame = {0};
            int frame = sensor->NuiSkeletonGetNextFrame(delay, &skeletonFrame);
            if (frame >= 0) {
                sensor->NuiTransformSmooth(&skeletonFrame, nullptr);

                // Loop over all sensed skeletons, stop at a valid person to be tracked
                for(const NUI_SKELETON_DATA & skeleton : skeletonFrame.SkeletonData) {
                    // Check the state of the skeleton
                    if (skeleton.eTrackingState == NUI_SKELETON_TRACKED) {
                        unique_lock<mutex> lck(mtx);

                        // Extract bone orientations
                        NuiSkeletonCalculateBoneOrientations(&skeleton, orientations);

                        // For the first tracked skeleton
                        // Copy the joint positions into our array
                        for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; i++) {
                            skeletonPositions[i] = skeleton.SkeletonPositions[i];

                            if (skeleton.eSkeletonPositionTrackingState[i] == NUI_SKELETON_POSITION_NOT_TRACKED) {
                                skeletonPositions[i].w = 0;
                            }
                        }

                        if(this->cache) {
                            consumed = false;
                            index++;

                            cv.notify_one();
                        }

                        // Only take the data for one skeleton
                        return true;
                    }
                }
            }
        }
    }

    void printJoints(int i, string joint_name1, string joint_name2, string joint_name3) {
        cout << "----------" << endl;
        string joint1 = joints[orientations[i].startJoint];
        string joint2 = joints[orientations[i].endJoint];
        if ((joint1 == joint_name1 && joint2 == joint_name2) ||
            (joint2 == joint_name1 && joint1 == joint_name2)) {
            printVec(orientations[i].absoluteRotation.rotationQuaternion);
        }

        string joint = joints[i];
        if (joint == joint_name1 || joint == joint_name2 || joint == joint_name3) {
            cout << joint << endl;

            printVec(skeletonPositions[i] );
        }
    }

    void copy(float * buffer) {
        for (unsigned int g = 0; g < getSize(); g++) {
            const Vector4 & joint = skeletonPositions[g];

            Device::copyVectorToBuffer(joint, buffer, g * 3);
        }
    }

    static void copyVectorToBuffer(const Vector4 & source, float * destination, unsigned int offset) {
        // Invalidate hidden joints
        if (source.w > 0) {
            destination[offset] = source.x;
            destination[offset + 1] = source.y;

            // The positive z axis is the visible area
            destination[offset + 2] = -source.z;
        } else {
            // Cull hidden & invalid joints
            destination[offset + 2] = 1.0;
        }
    }

    static void printVec(const Vector4 & source) {
        printf("vec: %f, %f, %f, %f\n", source.x, source.y, source.z, source.w);
    }

    void initCache(const string & path) {
        assert (frames > 0);

        this->cache = true;
        this->th = move(thread(&Device::writeToFile, this, path));
    }

    void clear() {
        sensor->Release();
    }

private:

    void writeToFile(const string & path) {
        string relativePath = "../output/" + path;

        fstream file(relativePath, fstream::out);
        if(file.is_open()) {
            printf("Writing to file: %d frames with %dms delay\n", frames, delay);

            while(true) {
                unique_lock<mutex> lck(mtx);

                if(index >= frames)
                    break;

                while(consumed) {
                    cv.wait(lck);
                }

                for (const Vector4 & joint : skeletonPositions) {
                    file << joint.x << " " << joint.y  << " " << joint.z  << " ";
                }

                file << "\n";

                for (const _NUI_SKELETON_BONE_ORIENTATION & orientation : orientations) {
                    file << orientation.startJoint << " " << orientation.endJoint << " ";

                    const Vector4 & quart = orientation.absoluteRotation.rotationQuaternion;
                    file << quart.x << " " << quart.y << " " << quart.z << " " << quart.w << "\n";
                }

                consumed = true;
            }

            file.flush();
            file.close();

            printf("Finished writing to file\n");
        } else {
            printf("Couldn't open %s\n", relativePath.c_str());
            exit(-1);
        }
    }
};

#endif