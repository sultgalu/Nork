# Prerequisites
- Windows
- Visual Studio 2022, c++23 features
- vcpkg integrated with VS
- Vulkan SDK

# Install Vulkan SDK
https://www.lunarg.com/vulkan-sdk/

# Install vcpkg
Steps are here: https://vcpkg.io/en/getting-started.html
- `git clone https://github.com/microsoft/vcpkg.git`
- `.\vcpkg\bootstrap-vcpkg.bat`
- `vcpkg integrate install`

# Setup

After opening the VS solution the first time, set Editor as startup project (right click on it and the option will appear)
Look out because the default build mode is Debug. It is recommended to build and run in Release mode for good performance.

When building Nork the first time, vcpkg automatically pulls and builds dependencies. After that you need to remove SPIRV-tools-shared.lib from vcpkg_repo and from under debug folder, otherwise you get a linking error (LNK1169-one or more multiply defined symbols found).
So delete these after the error appears: (vcpkg_repo\x64-windows\lib\SPIRV-Tools-shared.lib) (vcpkg_repo\x64-windows\debug\lib\SPIRV-Tools-shared.lib) 

The first build might take minutes because of building dependencies, and the project source is also quite big

The first run might take a few seconds, because of compiling shader sources, but the resulting binaries are cached so the following startups are faster. When you make changes in a shader' source code it will get recompiled (I save the hash of the source in the cached file). You can recompile them runtime in Graphics panel -> Recompile shaders

# Controls

Press F2 to Run physics simulation, press again to stop and the scene reloads

You can configure the default scene in Editor/Source/Main/Entry.cpp, InitScene()

To open an existing project: MenuBar -> File -> Open Project: select a .nork file

To change camera mode: inside Viewport panel, in left upper corner Camera -> Behaviour: select FPS or Editor. With Editor camera press F to focus on a selected entity.

# Notes
Only tested recently on the following system: Ryzen 5 1600, 16GB RAM, AMD Radeon RX 570 4GB VRAM
Previously tested on Intel HD GPU.

# Get Assets (not required)
`git clone https://github.com/KhronosGroup/glTF-Sample-Models`  
Nork implements a glTF loader, right now i'm using the above (2.0 folder) as an asset repo.
Note that these assets are for gltf testing and mostly contain edge-case scenarios, many of it is not supported by Nork (extensions, animations, ..), so not all models can be fully imported.
For a demo Sponza is the main go-to.
