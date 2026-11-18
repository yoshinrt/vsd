:@echo off

pushd ..

: ****************************************************************************
: *** build vsd_filter file

set path=c:\cygwin\bin;C:\strawberry\c\bin;C:\strawberry\perl\site\bin;C:\strawberry\perl\bin;%PATH%

if exist c:\cygwin\bin\ (
	echo cygwin detected
	
	c:\cygwin\bin\make.exe
) else (
	echo cygwin not detected
	
	perl make/make_js_func.pl ScriptIF.h
	perl make/set_git_version.pl rev_num.h
)

: ****************************************************************************
: *** build quickjs-ng

if exist quickjs\build_Win32\Release\qjs-libc.lib exit /b 0

cd quickjs
setlocal EnableDelayedExpansion

set "GENERATOR=Visual Studio 17 2022"

if not defined VSCMD_VER (
	for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSINSTALL=%%i"

	if not defined VSINSTALL (
		echo Visual Studio not found.
		exit /b 1
	)

	call "!VSINSTALL!\VC\Auxiliary\Build\vcvars64.bat"
	if errorlevel 1 exit /b 1
)

for %%A in (Win32 x64) do (
	rem Debug=/MDd, Release=/MD
	cmake -S . -B build_%%A ^
		-G "%GENERATOR%" ^
		-A %%A ^
		-DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
	if errorlevel 1 exit /b 1

	for %%C in (Debug Release) do (
		cmake --build build_%%A --config %%C --parallel
		if errorlevel 1 exit /b 1
	)

	rem ReleaseMT=/MT
	cmake -S . -B build_%%A_MT ^
		-G "%GENERATOR%" ^
		-A %%A ^
		-DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded"
	if errorlevel 1 exit /b 1

	cmake --build build_%%A_MT --config Release --parallel
	if errorlevel 1 exit /b 1
)

rmdir /s /q quickjs\build_Win32\ReleaseMT
rmdir /s /q quickjs\build_x64\ReleaseMT
move /y quickjs\build_Win32_MT\Release quickjs\build_Win32\ReleaseMT
move /y quickjs\build_x64_MT\Release quickjs\build_x64\ReleaseMT

endlocal
popd
