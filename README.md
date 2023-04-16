# Prerequisites
- Windows
- Visual Studio 2022
- Vulkan SDK
- vcpkg

# Install Vulkan SDK
https://www.lunarg.com/vulkan-sdk/

# Install vcpkg
Steps are here: https://vcpkg.io/en/getting-started.html
- `git clone https://github.com/microsoft/vcpkg.git`
- `.\vcpkg\bootstrap-vcpkg.bat`
- `vcpkg integrate install`

When building Nork the first time, vcpkg automatically pulls and builds dependencies. After that you need to remove SPIRV-tools-shared.lib from vc_repo and from under debug folder, otherwise you get a linking error.

# Get Assets
`git clone https://github.com/KhronosGroup/glTF-Sample-Models`  
Nork implements a glTF loader, right now i'm using the above (2.0 folder) as an asset repo.
Note that these assets are for gltf testing and mostly contain edge-case scenarios, many of it is not supported by Nork (extensions, animations, ..), so not all models can be fully imported.
For a demo Sponza is the main go-to.
