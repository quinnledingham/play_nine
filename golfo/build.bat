@echo off

set SDL_SDK=..\libs\SDL3-3.2.6
set VULKAN_SDK=..\libs\VulkanSDK\1.3.296.0

set OS=WINDOWS

set CF_SDL=/I%SDL_SDK%\include\SDL3 /I%SDL_SDK%\include -DSDL
set CF_VULKAN=/I%VULKAN_SDK%\Include 
set CF=-nologo -Z7 -W3 -MD %CF_SDL% %CF_VULKAN% /Fe:golfo.exe /I..\libs\ -D_CRT_SECURE_NO_WARNINGS

set LF_SDL=%SDL_SDK%\lib\x64\SDL3.lib
set LF_VULKAN=%VULKAN_SDK%\Lib\vulkan-1.lib 
set LF_SHADERC=%VULKAN_SDK%\Lib\spirv-cross-c-shared.lib %VULKAN_SDK%\Lib\shaderc_combined.lib
set LF=-incremental:no -subsystem:windows %LF_SDL% %LF_VULKAN% %LF_SHADERC%

cls

:: create/move into build directory
pushd %CD%
IF NOT EXIST build mkdir build
cd build

:: copy dlls if needed
IF NOT EXIST SDL3.dll copy %SDL_SDK%\lib\x64\SDL3.dll

:: COMPILE
cl %CF% ../sdl_golfo.cpp /link %LF%

:: move out of build directory
cd ..
