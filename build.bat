@echo off

REM create build directory
pushd %CD%
IF NOT EXIST build mkdir build
cd build

REM CF = Compiler Flags
REM LF = Linker Flags

REM Change these directories to where the SDKs are on your computer
set SDL_SDK= ..\sdl-vc
set VULKAN_SDK= ..\VulkanSDK\1.3.268.0
set STEAM_SDK= ..\steam

set CF_DEFAULT= -MD -nologo -Gm- -GR- -EHa- -Od -Oi -FC -Z7 -W3 -EHsc -D_CRT_SECURE_NO_WARNINGS /I..\stb /I..\ /I..\qlib -DSHADERS -DOS_WINDOWS -DDEBUG
set CF_SDL= /I%SDL_SDK%\include -DSDL
set CF_OPENGL= /I..\glad -DOPENGL
set CF_VULKAN= /I%VULKAN_SDK%\Include -DVULKAN
set CF_STEAM= /I%STEAM_SDK% -DSTEAM2

set LF_DEFAULT= -incremental:no -opt:ref -subsystem:windows
set LF_SDL= shell32.lib %SDL_SDK%\lib\x64\SDL2main.lib %SDL_SDK%\lib\x64\SDL2.lib
set LF_SHADERS= %VULKAN_SDK%\Lib\spirv-cross-c-shared.lib %VULKAN_SDK%\Lib\shaderc_combined.lib
set LF_OPENGL= opengl32.lib 
set LF_VULKAN= %VULKAN_SDK%\Lib\vulkan-1.lib
set LF_DX12= D3d12.lib D3DCompiler.lib dxgi.lib
set LF_STEAM= %STEAM_SDK%\redistributable_bin\win64\steam_api64.lib

set V_CF= %CF_DEFAULT% %CF_SDL% %CF_VULKAN% %CF_STEAM%  
set V_LF= %LF_DEFAULT% %LF_SDL% %LF_VULKAN% %LF_SHADERS% %LF_STEAM%

cl %V_CF% ../play_nine.cpp /link %V_LF% /out:play_nine.exe
REM cl %CF_DEFAULT% %CF_SDL% %CF_OPENGL% %CF_VULKAN% %CF_STEAM% -DOS_WINDOWS -DDEBUG2 ../glad/gl.c ../qlib/sdl_application.cpp /link %LF_DEFAULT% %LF_SDL% %LF_OPENGL% %LF_SHADERS% %LF_STEAM% /out:play_nine.exe

IF NOT EXIST SDL2.dll copy %SDL_SDK%\lib\x64\SDL2.dll

cd ..
