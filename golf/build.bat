@echo off

REM create build directory
pushd %CD%
IF NOT EXIST build mkdir build
cd build

set CF= -MD -nologo -Gm- -GR- -EHa- -Od -Oi -FC -Z7 -W3 -EHsc -D_CRT_SECURE_NO_WARNINGS

set LF_DX12= D3d12.lib D3DCompiler.lib dxgi.lib
set LF= -incremental:no -opt:ref -subsystem:windows user32.lib %LF_DX12%

cl %CF% -I../dx12 -I../stb -DDEBUG ../win32.cpp /link %LF% /out:golf.exe

cd ..
