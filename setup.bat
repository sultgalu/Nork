@echo off

::IF "%1"=="-h" echo "USAGE: pass cmake exe path (or nothing if it is in system path)" & EXIT /B
echo ------------SETUP START-------------
echo -------------INITIALIZING SUBMODULES-------------
@echo on
git submodule update --init --recursive
@echo off
echo -------------SUBMODULES DONE-------------
echo ------------SETUP END-------------