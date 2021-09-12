@echo off
set extension=.spv
cd shaders/
for %%i in (*) do (
	echo compiling %%~fi...
	"glslc/glslc.exe" %%~fi -o %%~ni.spv
)