@echo off

:: Load local variables

:: Default Values
set VARS=SDL_SDK VULKAN_SDK OS SDL_TTF SDL_IMAGE

set SDL_SDK=C:\Users\quinn\libs\SDL3-3.2.4
set SDL_TTF=C:\Users\quinn\libs\SDL3_ttf-3.1.0
set SDL_IMAGE=C:\Users\quinn\libs\SDL3_image-3.2.0
set VULKAN_SDK=C:\VulkanSDK\1.3.296.0
set STEAM_SDK=S:\play_nine\rewrite\libs\steamworks_sdk
set OS=WINDOWS

IF EXIST local.bat call local.bat
for %%V in (%VARS%) do (
	if not defined %%V (echo %%V is not defined & exit /b)
)

set CF=-nologo -Z7 -W3 -MD -Gm- -GR- -DDEBUG -DOS_%OS% /std:c++20 /EHsc -DGFX_VULKAN /I..\libs\ -D_CRT_SECURE_NO_WARNINGS
set CF_SDL=/I%SDL_SDK%\include\SDL3 /I%SDL_SDK%\include -DSDL /I%SDL_TTF%\include /I%SDL_IMAGE%\include
set CF_VULKAN=/I%VULKAN_SDK%\Include 
set CF_STEAM=/I%STEAM_SDK%\public\steam

set LF=-incremental:no -opt:ref -subsystem:console
set LF_SDL=shell32.lib  %SDL_SDK%\lib\x64\SDL3.lib %SDL_TTF%\lib\x64\SDL3_ttf.lib %SDL_IMAGE%\lib\x64\SDL3_image.lib
set LF_VULKAN=%VULKAN_SDK%\Lib\vulkan-1.lib 
set LF_SHADERC=%VULKAN_SDK%\Lib\spirv-cross-c-shared.lib %VULKAN_SDK%\Lib\shaderc_combined.lib
set LF_STEAM=%STEAM_SDK%\redistributable_bin\win64\steam_api64.lib

:: create build directory
pushd %CD%
IF NOT EXIST build mkdir build
cd build

:: copy sdl dll if needed
IF NOT EXIST SDL3.dll copy %SDL_SDK%\lib\x64\SDL3.dll
IF NOT EXIST SDL3_ttf.dll copy %SDL_TTF%\lib\x64\SDL3_ttf.dll
IF NOT EXIST SDL3_image.dll copy %SDL_IMAGE%\lib\x64\SDL3_image.dll

:: copy steam dll if needed
IF NOT EXIST steam_api64.dll copy %STEAM_SDK%\redistributable_bin\win64\steam_api64.dll

cl %CF% %CF_SDL% %CF_VULKAN% %CF_STEAM% ../play.cpp /link %LF% %LF_SDL% %LF_VULKAN% %LF_SHADERC% %LF_STEAM% /out:play.exe

cd ..

:: Unset all local variables
for %%V in (%VARS%) do set %%V=
