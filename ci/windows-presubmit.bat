SETLOCAL ENABLEDELAYEDEXPANSION

SET BAZEL_EXE=%KOKORO_GFILE_DIR%\bazel-5.1.1-windows-x86_64.exe

SET PATH=C:\Python37;%PATH%
SET BAZEL_PYTHON=C:\python37\python.exe
SET BAZEL_SH=C:\tools\msys64\usr\bin\bash.exe
SET CMAKE_BIN="C:\Program Files\CMake\bin\cmake.exe"
SET CTEST_BIN="C:\Program Files\CMake\bin\ctest.exe"
SET CTEST_OUTPUT_ON_FAILURE=1

IF EXIST git\googletest (
  CD git\googletest
) ELSE IF EXIST github\googletest (
  CD github\googletest
)

IF %errorlevel% neq 0 EXIT /B 1

:: ----------------------------------------------------------------------------
:: CMake Visual Studio 15 2017 Win64
MKDIR cmake_msvc2017
CD cmake_msvc2017

%CMAKE_BIN% .. ^
  -G "Visual Studio 15 2017 Win64" ^
  -DPYTHON_EXECUTABLE:FILEPATH=c:\python37\python.exe ^
  -DPYTHON_INCLUDE_DIR:PATH=c:\python37\include ^
  -DPYTHON_LIBRARY:FILEPATH=c:\python37\lib\site-packages\pip ^
  -Dgtest_build_samples=ON ^
  -Dgtest_build_tests=ON ^
  -Dgmock_build_tests=ON
IF %errorlevel% neq 0 EXIT /B 1

%CMAKE_BIN% --build . --target ALL_BUILD --config Debug -- -maxcpucount
IF %errorlevel% neq 0 EXIT /B 1

%CTEST_BIN% -C Debug --timeout 600
IF %errorlevel% neq 0 EXIT /B 1

CD ..
RMDIR /S /Q cmake_msvc2017

:: ----------------------------------------------------------------------------
:: Bazel Visual Studio 15 2017 Win64

SET BAZEL_VC=C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC
%BAZEL_EXE% test ... ^
  --compilation_mode=dbg ^
  --copt=/std:c++14 ^
  --copt=/WX ^
  --features=external_include_paths ^
  --keep_going ^
  --test_output=errors ^
  --test_tag_filters=-no_test_msvc2017
IF %errorlevel% neq 0 EXIT /B 1
