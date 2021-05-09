@echo off
echo starting CMake...
cmake -H. -Bbuild -G "Visual Studio 16 2019" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/Users/Evan/Desktop/vpkg/vcpkg/scripts/buildsystems/vcpkg.cmake
echo done!
echo press any key to contunue.
pause > nul