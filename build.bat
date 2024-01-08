@echo off

REM create build directory
pushd %CD%
IF NOT EXIST build mkdir build
cd build

REM CF = Compiler Flags
REM LF = Linker Flags

set CF_WINDOWS_SDL_OPENGL= -MTd -nologo -Gm- -GR- -EHa- -Od -Oi -FC -Z7 -W3 /I..\sdl-vc\include /I..\glad /I..\stb /DWINDOWS /DOPENGL /DSDL /D_CRT_SECURE_NO_WARNINGS
set CF_WINDOWS_DX12= -MTd -nologo -Gm- -GR- -EHa- -Od -Oi -FC -Z7 -EHsc /I..\stb /DWINDOWS /D_CRT_SECURE_NO_WARNINGS -W3 
REM set CFs2= -nologo -O2 -Oi /Isdl-vc\include /Iglad /Istb /DWINDOWS

set LF_WINDOWS_SDL_OPENGL= -incremental:no -opt:ref shell32.lib opengl32.lib ..\sdl-vc\lib\x64\SDL2main.lib ..\sdl-vc\lib\x64\SDL2.lib /subsystem:windows
set LF_WINDOWS_DX12= -incremental:no -opt:ref shell32.lib user32.lib gdi32.lib D3d12.lib D3DCompiler.lib dxgi.lib /subsystem:windows

cl %CF_WINDOWS_SDL_OPENGL% ../play_nine.cpp ../sdl_application.cpp /link %LF_WINDOWS_SDL_OPENGL% /out:play_nine.exe
REM cl %CF_WINDOWS_DX12% /DDX12 ../win32_application.cpp /link %LF_WINDOWS_DX12% /out:river.exe

REM cl %CFs% ../assets_builder.cpp /link %LFs% /out:builder.exe

IF NOT EXIST SDL2.dll copy ..\sdl-vc\lib\x64\SDL2.dll build