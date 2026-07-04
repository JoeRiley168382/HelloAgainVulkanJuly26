@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=amd64
if errorlevel 1 goto :fail

if exist Build rmdir /s /q Build

cmake -S . -B Build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DVulkan_INCLUDE_DIR="C:/VulkanSDK/1.4.350.0/Include" -DVulkan_LIBRARY="C:/VulkanSDK/1.4.350.0/Lib/vulkan-1.lib"
if errorlevel 1 goto :fail

cmake --build Build
if errorlevel 1 goto :fail

echo Build succeeded.
goto :eof

:fail
echo Build FAILED.
exit /b 1
