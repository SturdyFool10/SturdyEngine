@echo off
del output.txt
for /r %%i in (*.lib) do (
	echo includelib/%%~ni  >> output.txt
)