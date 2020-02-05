<h4 align="center">Automatic 3D model rigging and real-time Linear Mesh Blending avatar animation in C++ and Kinect.</h4>

<h1 align="center">
    <img src="https://raw.githubusercontent.com/DomainFlag/automatic_rigging_animation/master/Assets/animation.gif" alt="LBS animation" width=60%>
</h1>

## Key Features

* Kinect animation and caching
* 3D model animation based on LBS
* Automatic skeleton and bone weights generation
* Support for various 3D models: .obj .fbx .off, .ply and .stl file formats.
* Cross platform
  - Windows (only Kinect), macOS and Linux ready.

## Dependencies

The project was built / run on Windows, Visual Studio 2019. The [Desktop development with C++ bundle](https://visualstudio.microsoft.com/downloads/)
must include MSVC v142 - VS 2019 build tools, Windows 10 SDK, C++ MFC for MSVC v142.

- [FLTK](https://www.fltk.org) - library v1.3.5 is needed for rendering. Download and unzip it and follow
                              the [instructions](http://www.c-jump.com/bcc/common/Talk2/Cxx/FltkInstallVC/FltkInstallVC.html), set the appropriate env `FLTK_DIR`.

- [STB](https://github.com/nothings/stb) - is required for image texture loading. Clone the [project](https://github.com/nothings/stb),
                                           as it is a header library, set only the appropriate env `STB_DIR`.

- [Eigen](https://gitlab.com/libeigen/eigen) - is used for linear algebra related problems. Clone the [project](https://gitlab.com/libeigen/eigen),
                                               as it is a header library, set only the appropriate env `EIGEN_DIR`.

- [Kinect for Windows SDK v1.8](https://www.microsoft.com/en-us/download/details.aspx?id=40278) - allows skeleton tracking to cache animations and live animate a 3D model.
    Download and install the distribution and set `KINECTSDK10_DIR` if it's not defined to the installation path.

- [FBX SDK](https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2019-0) - is useful to load complex 3D models that contain textures, skeleton...
    Download and install the distribution for Windows and set `FBX_DIR` to the installation path or follow the following [instructions](http://help.autodesk.com/view/FBX/2017/ENU/?guid=__files_GUID_6E7B1A2E_584C_49C6_999E_CD8367841B7C_htm).

Older releases may work too...

## How To Use

Loading a 3D model that is automatically rigged based on the joints.out and animate live based on Kinect:

`data/human.obj data/joints.out data/skeleton.out non-rigged live`

Loading a 3D model that is rigged and animated based on cached animation skeleton.out:

`data/model.fbx data/joints.out data/skeleton.out rigged non-live`

Note: Run in release mode to speed up things. To set up the command arguments in VS2019, select `Project project > Properties > Configuration Properties > Debugging > Command Arguments`

## Interaction

- Press `s` to toggle skeleton, `g` for floor, `f` for flat shading rendering.
- Use mouse wheel or by mouse dragging to move the camera.

## Miscellaneous

The file `data/joints.out` contain the definition of the desired base skeleton (human, monkey, koala...). The loaded 3D model
is embedded / scaled into [-1, 1] world space, thus each line contains an unique joint, its desired / expected 3D location,
and his parent joint. Specify -1 if the corresponding joint is the root.

The motion file `data/skeleton.out` is based on our custom motion format, where the skeleton
is tracked each frame with first line representing [20 joints (Kinect v1)](https://de.mathworks.com/help/supportpkg/kinectforwindowsruntime/ug/use-skeleton-viewer-for-kinect-v1-skeletal-data.html)
world positions and 20 new lines each representing the `index of parent joint`, `index of child joint`, `absolute quaternion rotation w.t.r Y-axis for joints world orientation`

## Credits

The rigging is based on:

- [Pinocchio](https://github.com/elrond79/Pinocchio) - please refer also the paper for extra information about rigging process.
