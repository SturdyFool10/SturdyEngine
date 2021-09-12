set startDir=%cd%
call shaders/compileShaders.bat
cd %startDir%
robocopy shaders/ Release/shaders/ /E
robocopy shaders/ Debug/shaders/ /E
pause