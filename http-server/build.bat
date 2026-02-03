@echo off
mkdir build 2>nul
cd build
cmake ..
cmake --build . --config Release
echo Build complete! Run with: httpserver.exe