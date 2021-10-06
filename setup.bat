@echo off

IF "%1"=="" echo "USAGE: pass cmake bin folder path (or nothing if it is in system path)" & EXIT /B
echo ------------SETUP START-------------
echo -------------INITIALIZING SUBMODULES-------------
@echo on
git submodule update --init --recursive
echo -------------SUBMODULES DONE-------------
echo ------------BUILDING ASSIMP----------------
.\Nork\Source\ThirdParty\include\assimp\build %1
echo ------------ASSIMP DONE-------------
echo ------------SETUP END-------------