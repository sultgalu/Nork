#install vulkan
https://www.lunarg.com/vulkan-sdk/

#install vcpkg
git clone
call script
vcpkg integrate install
// set environment variable *VCPKG_DEFAULT_TRIPLET* to *x64-windows* (by default it installs x86 deps)
after build remove SPIRV-tools-shared.lib from vc_repo (also from under debug folder)

#Get Assets
git clone https://github.com/KhronosGroup/glTF-Sample-Models
Nork implements a glTF loader, right now i'm using the above (2.0) as an asset repo.
Note that that repo is full of edge-case scenarios, many of it is not supported by Nork, so not all models can be fully imported.
For a demo Sponza is the main go-to.