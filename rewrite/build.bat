@echo off

set SDL_SDK=C:\Users\quinn\libs\SDL3-3.2.4
set SDL_TTF=C:\Users\quinn\libs\SDL3_ttf-3.1.0
set SDL_IMAGE=C:\Users\quinn\libs\SDL3_image-3.2.0

set VULKAN_SDK=C:\VulkanSDK\1.3.296.0
set OS=WINDOWS

set CF=-nologo -Z7 -W0 -MD -Gm- -GR- -DDEBUG -DOS_%OS% /std:c++20 /EHsc -DGFX_VULKAN /I..\libs\
set CF_SDL=/I%SDL_SDK%\include\SDL3 /I%SDL_SDK%\include -DSDL /I%SDL_TTF%\include /I%SDL_IMAGE%\include
set CF_VULKAN=/I%VULKAN_SDK%\Include 

set LF=-incremental:no -opt:ref -subsystem:console
set LF_SDL=shell32.lib  %SDL_SDK%\lib\x64\SDL3.lib %SDL_TTF%\lib\x64\SDL3_ttf.lib %SDL_IMAGE%\lib\x64\SDL3_image.lib
set LF_VULKAN=%VULKAN_SDK%\Lib\vulkan-1.lib 
set LF_SHADERC=%VULKAN_SDK%\Lib\spirv-cross-c-shared.lib %VULKAN_SDK%\Lib\shaderc_combined.lib

REM create build directory
pushd %CD%
IF NOT EXIST build mkdir build
cd build

REM copy sdl dll if needed
IF NOT EXIST SDL3.dll copy %SDL_SDK%\lib\x64\SDL3.dll
IF NOT EXIST SDL3_ttf.dll copy %SDL_TTF%\lib\x64\SDL3_ttf.dll
IF NOT EXIST SDL3_image.dll copy %SDL_IMAGE%\lib\x64\SDL3_image.dll


cl %CF% %CF_SDL% %CF_VULKAN% ../play_nine.cpp ../clay/clay_iru.c /link %LF% %LF_SDL% %LF_VULKAN% %LF_SHADERC% /out:play.exe

cd ..
