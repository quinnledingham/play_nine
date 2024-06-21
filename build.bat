@echo off

REM create build directory
pushd %CD%
IF NOT EXIST build mkdir build
cd build

REM CF = Compiler Flags
REM LF = Linker Flags

set CF_DEFAULT= -MD -nologo -Gm- -GR- -EHa- -Od -Oi -FC -Z7 -W3 -EHsc -D_CRT_SECURE_NO_WARNINGS /I..\stb /I..\ -DSHADERS
set CF_SDL= /I..\sdl-vc\include -DSDL
set CF_OPENGL= /I..\glad -DOPENGL
set CF_VULKAN= /I..\VulkanSDK\1.3.268.0\Include -DVULKAN
set CF_STEAM= /I..\steam -DSTEAM

set LF_DEFAULT= -incremental:no -opt:ref -subsystem:windows
set LF_SDL= shell32.lib ..\sdl-vc\lib\x64\SDL2main.lib ..\sdl-vc\lib\x64\SDL2.lib
set LF_SHADERS= ..\VulkanSDK\1.3.268.0\Lib\spirv-cross-c-shared.lib ..\VulkanSDK\1.3.268.0\Lib\shaderc_combined.lib
set LF_OPENGL= opengl32.lib 
set LF_VULKAN= ..\VulkanSDK\1.3.268.0\Lib\vulkan-1.lib
set LF_DX12= D3d12.lib D3DCompiler.lib dxgi.lib
set LF_STEAM= ..\steam\redistributable_bin\win64\steam_api64.lib

cl %CF_DEFAULT% %CF_SDL% %CF_VULKAN% %CF_STEAM% -DOS_WINDOWS -DDEBUG ../qlib/sdl_application.cpp /link %LF_DEFAULT% %LF_SDL% %LF_VULKAN% %LF_SHADERS% %LF_STEAM% /out:play_nine.exe
REM cl %CF_DEFAULT% %CF_SDL% %CF_OPENGL% %CF_VULKAN% %CF_STEAM% -DOS_WINDOWS -DDEBUG ../glad/gl.c ../qlib/sdl_application.cpp /link %LF_DEFAULT% %LF_SDL% %LF_OPENGL% %LF_SHADERS% %LF_STEAM% /out:play_nine.exe

IF NOT EXIST SDL2.dll copy ..\sdl-vc\lib\x64\SDL2.dll

cd ..
