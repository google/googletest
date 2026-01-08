SETLOCAL ENABLEDELAYEDEXPANSION

SET BAZEL_EXE=%KOKORO_GFILE_DIR%\bazel-8.2.1-windows-x86_64.exe

SET PATH=C:\Python34;%PATH%
SET BAZEL_PYTHON=C:\python34\python.exe
SET BAZEL_SH=C:\tools\msys64\usr\bin\bash.exe
SET CMAKE_BIN="cmake.exe"
SET CTEST_BIN="ctest.exe"
SET CTEST_OUTPUT_ON_FAILURE=1
SET CMAKE_BUILD_PARALLEL_LEVEL=16
SET CTEST_PARALLEL_LEVEL=16

SET GTEST_ROOT=%~dp0\..
IF %errorlevel% neq 0 EXIT /B 1

:: ----------------------------------------------------------------------------
:: CMake
SET CMAKE_BUILD_PATH=cmake_msvc2022
MKDIR %CMAKE_BUILD_PATH%
CD %CMAKE_BUILD_PATH%

%CMAKE_BIN% %GTEST_ROOT% ^
  -G "Visual Studio 17 2022" ^
  -DCMAKE_CXX_STANDARD=17 ^
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

CD %GTEST_ROOT%
RMDIR /S /Q %CMAKE_BUILD_PATH%

:: ----------------------------------------------------------------------------
:: Bazel

:: The default home directory on Kokoro is a long path which causes errors
:: because of Windows limitations on path length.
:: --output_user_root=C:\tmp causes Bazel to use a shorter path.
SET BAZEL_VS=C:\Program Files\Microsoft Visual Studio\2022\Community

:: Use Bazel Vendor mode to reduce reliance on external dependencies.
IF EXIST "%KOKORO_GFILE_DIR%\distdir\googletest_vendor.tar.gz" (
  tar --force-local -xf "%KOKORO_GFILE_DIR%\distdir\googletest_vendor.tar.gz" -C c:
  SET VENDOR_FLAG=--vendor_dir=c:\googletest_vendor
) ELSE (
  SET VENDOR_FLAG=
)

:: C++17
%BAZEL_EXE% ^
  --output_user_root=C:\tmp ^
  test ... ^
  --compilation_mode=dbg ^
  --copt=/std:c++17 ^
  --copt=/WX ^
  --enable_bzlmod=true ^
  --keep_going ^
  --per_file_copt=external/.*@/w ^
  --test_output=errors ^
  --test_tag_filters=-no_test_msvc2017 ^
  %VENDOR_FLAG%
IF %errorlevel% neq 0 EXIT /B 1

:: C++20
%BAZEL_EXE% ^
  --output_user_root=C:\tmp ^
  test ... ^
  --compilation_mode=dbg ^
  --copt=/std:c++20 ^
  --copt=/WX ^
  --enable_bzlmod=true ^
  --keep_going ^
  --per_file_copt=external/.*@/w ^
  --test_output=errors ^
  --test_tag_filters=-no_test_msvc2017 ^
  %VENDOR_FLAG%
IF %errorlevel% neq 0 EXIT /B 1
