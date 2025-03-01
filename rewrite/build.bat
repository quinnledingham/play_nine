@echo off

set SDL_SDK=C:\Users\quinn\libs\SDL2-devel-2.30.8-VC
set VULKAN_SDK=C:\VulkanSDK\1.3.296.0
set OS=WINDOWS

set CF=-nologo -Z7 -W3 -MD -Gm- -GR- -DDEBUG -DOS_%OS%
set CF_SDL=/I%SDL_SDK%\include -DSDL
set CF_VULKAN=/I%VULKAN_SDK%\Include -DAPI3D_VULKAN

set LF=-incremental:no -opt:ref -subsystem:windows
set LF_SDL=shell32.lib %SDL_SDK%\lib\x64\SDL2main.lib %SDL_SDK%\lib\x64\SDL2.lib
set LF_VULKAN=%VULKAN_SDK%\Lib\vulkan-1.lib 
set LF_SHADERC=%VULKAN_SDK%\Lib\spirv-cross-c-shared.lib %VULKAN_SDK%\Lib\shaderc_combined.lib

REM create build directory
pushd %CD%
IF NOT EXIST build mkdir build
cd build

REM copy sdl dll if needed
IF NOT EXIST SDL2.dll copy %SDL_SDK%\lib\x64\SDL2.dll

cl %CF% %CF_SDL% %CF_VULKAN% ../play_nine.cpp /link %LF% %LF_SDL% %LF_VULKAN% %LF_SHADERC% /out:play.exe

cd ..
