@echo off
setlocal
cd /d %~dp0

rem -- Assume the default installation path for Visual Studio unless we can find it.
rem -- (If the user is running this from a "Developer Command Prompt" then use the devenv.exe
rem -- in the path.)
where devenv.exe 2> NUL
if ERRORLEVEL 1 (
    set DEVENV="C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE\devenv.exe"
) else (
    for /f "tokens=*" %%d in ('where devenv.exe') do set DEVENV="%%d"
)

set EXITCODE=0

call :BuildSln gmock Debug Win32
call :BuildSln gmock Release Win32
call :BuildSln gmock Debug x64
call :BuildSln gmock Release x64

if %EXITCODE% EQU 0 (
    echo Running tests
    for /f %%x in ('dir /b /s *.exe') do (
        echo Executing %%x
        %%x > NUL
        if ERRORLEVEL 1 set EXITCODE=2
    )
)

echo Finished -- %TIME%
exit /b %EXITCODE%

:BuildSln
    set SOLUTION=%1.sln
    set OUTFILE="_build-%1-%2-%3.txt"
    if EXIST %OUTFILE% del %OUTFILE% > NUL
    echo Building %SOLUTION% %2^|%3 -- %TIME%
    %DEVENV% %SOLUTION% /Build "%2|%3" /Out %OUTFILE%
    if ERRORLEVEL 1 (
        echo Build of %SOLUTION% failed for %2^|%3
        set EXITCODE=1
    )
    goto :eof

