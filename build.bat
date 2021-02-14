
@REM @echo off

setlocal

set VS2019TOOLS="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
@REM if not exist %VS2017TOOLS% (
@REM     echo "VS 2017 Build Tools are missing!"
@REM     exit
@REM )
call %VS2019TOOLS%


set include_flags=-I..\include\
@REM /Yc"pch.h" /Fp"procAnim.pch" /Fa"."  /Fo"." /nologo
set compiler_flags= /Od /W4 /WX /permissive- /std:c++17 /ZI /sdl /Zc:inline /Fe"procAnim.exe"   /EHsc  %include_flags%
@REM set compiler_flags=/JMC /permissive- /Yu"pch.h" /ifcOutput "D:\Dev\procAnim\\bin\" /GS /W4 /Zc:wchar_t /I"D:\Dev\procAnim\\include" /ZI /Gm- /Od /sdl /Fd"D:\Dev\procAnim\\bin\vc142.pdb" /Zc:inline /fp:precise /D "_MBCS" /errorReport:prompt /WX /Zc:forScope /RTC1 /Gd /MDd /std:c++17 /FC /Fa"D:\Dev\procAnim\\bin\" /EHsc /nologo /Fo"D:\Dev\procAnim\\bin\" /Fp"D:\Dev\procAnim\\bin\procAnim.pch" /diagnostics:column

set linker_flags=/OUT:"procAnim.exe" /PDB:"procAnim.pdb" /DEBUG:FASTLINK /MACHINE:X64 /SUBSYSTEM:CONSOLE /NOLOGO /LIBPATH:"..\lib" "OpenGL32.lib" "glew32.lib" "SDL2.lib" "SDL2main.lib" "SDL2_image.lib" "SDL2_ttf.lib" "SDL2_mixer.lib" "assimp-vc142-mt.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib"
@REM set linker_flags=/OUT:"D:\Dev\procAnim\\bin\procAnim.exe" /MANIFEST /NXCOMPAT /PDB:"D:\Dev\procAnim\\bin\procAnim.pdb" /DYNAMICBASE "OpenGL32.lib" "glew32.lib" "SDL2.lib" "SDL2main.lib" "SDL2_image.lib" "SDL2_ttf.lib" "SDL2_mixer.lib" "assimp-vc142-mt.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" /DEBUG:FASTLINK /MACHINE:X64 /INCREMENTAL /PGD:"D:\Dev\procAnim\\bin\procAnim.pgd" /SUBSYSTEM:CONSOLE /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /ManifestFile:"D:\Dev\procAnim\\bin\procAnim.exe.intermediate.manifest" /ERRORREPORT:PROMPT /NOLOGO /LIBPATH:"D:\Dev\procAnim\\lib" /TLBID:1 


pushd bin
    cl %compiler_flags% "..\\src\unity.cpp" /link %linker_flags%  
popd

@REM Maybe add later:
@REM /MDd /D "_MBCS"

@REM VS compiler command line:
@REM /JMC /permissive- /Yu"pch.h" /ifcOutput "D:\Dev\procAnim\\bin\" /GS /W4 /Zc:wchar_t /I"D:\Dev\procAnim\\include" /ZI /Gm- /Od /sdl /Fd"D:\Dev\procAnim\\bin\vc142.pdb" /Zc:inline /fp:precise /D "_MBCS" /errorReport:prompt /WX /Zc:forScope /RTC1 /Gd /MDd /std:c++17 /FC /Fa"D:\Dev\procAnim\\bin\" /EHsc /nologo /Fo"D:\Dev\procAnim\\bin\" /Fp"D:\Dev\procAnim\\bin\procAnim.pch" /diagnostics:column 

@REM VS linker command line:
@REM /OUT:"D:\Dev\procAnim\\bin\procAnim.exe" /MANIFEST /NXCOMPAT /PDB:"D:\Dev\procAnim\\bin\procAnim.pdb" /DYNAMICBASE "OpenGL32.lib" "glew32.lib" "SDL2.lib" "SDL2main.lib" "SDL2_image.lib" "SDL2_ttf.lib" "SDL2_mixer.lib" "assimp-vc142-mt.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" /DEBUG:FASTLINK /MACHINE:X64 /INCREMENTAL /PGD:"D:\Dev\procAnim\\bin\procAnim.pgd" /SUBSYSTEM:CONSOLE /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /ManifestFile:"D:\Dev\procAnim\\bin\procAnim.exe.intermediate.manifest" /ERRORREPORT:PROMPT /NOLOGO /LIBPATH:"D:\Dev\procAnim\\lib" /TLBID:1 