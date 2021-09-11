@echo off
echo starting CMake...
cmake -H. -Bbuild -G "Visual Studio 16 2019" -A x64
echo done!
echo press any key to contunue.
pause > nul