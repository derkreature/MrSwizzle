@echo off
setlocal
set PROJECT=MrSwizzle

mkdir Build64
cd Build64

cmake.exe -Wno-dev -G "Visual Studio 14 2015 Win64" ../

endlocal

pause
