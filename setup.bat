echo ------------SETUP START-------------
echo -------------INITIALIZING SUBMODULES-------------
git submodule update --init --recursive
echo -------------SUBMODULES DONE-------------
echo ------------BUILDING ASSIMP----------------
.\Nork\Source\ThirdParty\include\assimp\build
echo ------------ASSIMP DONE-------------
echo ------------SETUP END-------------