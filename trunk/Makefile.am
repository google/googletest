# Automake file

# TODO(chandlerc@google.com): automate the generation of *.h from *.h.pump.

# Nonstandard package files for distribution
EXTRA_DIST = \
  CHANGES \
  CONTRIBUTORS \
  include/gtest/gtest-param-test.h.pump \
  include/gtest/internal/gtest-tuple.h.pump \
  include/gtest/internal/gtest-type-util.h.pump \
  include/gtest/internal/gtest-param-util-generated.h.pump \
  make/Makefile \
  scripts/fuse_gtest_files.py \
  scripts/gen_gtest_pred_impl.py \
  scripts/test/Makefile

# gtest source files that we don't compile directly.
EXTRA_DIST += \
  src/gtest.cc \
  src/gtest-death-test.cc \
  src/gtest-filepath.cc \
  src/gtest-internal-inl.h \
  src/gtest-port.cc \
  src/gtest-test-part.cc \
  src/gtest-typed-test.cc

# Sample files that we don't compile.
EXTRA_DIST += \
  samples/prime_tables.h \
  samples/sample2_unittest.cc \
  samples/sample3_unittest.cc \
  samples/sample4_unittest.cc \
  samples/sample5_unittest.cc \
  samples/sample6_unittest.cc \
  samples/sample7_unittest.cc \
  samples/sample8_unittest.cc \
  samples/sample9_unittest.cc

# C++ test files that we don't compile directly.
EXTRA_DIST += \
  test/gtest-death-test_test.cc \
  test/gtest_environment_test.cc \
  test/gtest-filepath_test.cc \
  test/gtest-linked_ptr_test.cc \
  test/gtest-message_test.cc \
  test/gtest_no_test_unittest.cc \
  test/gtest-options_test.cc \
  test/gtest-param-test_test.cc \
  test/gtest-param-test2_test.cc \
  test/gtest-param-test_test.h \
  test/gtest-port_test.cc \
  test/gtest_pred_impl_unittest.cc \
  test/gtest_prod_test.cc \
  test/production.cc \
  test/production.h \
  test/gtest_repeat_test.cc \
  test/gtest_sole_header_test.cc \
  test/gtest_stress_test.cc \
  test/gtest-test-part_test.cc \
  test/gtest_throw_on_failure_ex_test.cc \
  test/gtest-typed-test_test.cc \
  test/gtest-typed-test2_test.cc \
  test/gtest-typed-test_test.h \
  test/gtest_unittest.cc \
  test/gtest-unittest-api_test.cc \
  test/gtest-listener_test.cc \
  test/gtest_main_unittest.cc \
  test/gtest_unittest.cc \
  test/gtest-tuple_test.cc \
  test/gtest-param-test_test.cc \
  test/gtest-param-test2_test.cc \
  test/gtest_break_on_failure_unittest_.cc \
  test/gtest_color_test_.cc \
  test/gtest_env_var_test_.cc \
  test/gtest_filter_unittest_.cc \
  test/gtest_help_test_.cc \
  test/gtest_list_tests_unittest_.cc \
  test/gtest_output_test_.cc \
  test/gtest_shuffle_test_.cc \
  test/gtest_throw_on_failure_test_.cc \
  test/gtest_uninitialized_test_.cc \
  test/gtest_xml_outfile1_test_.cc \
  test/gtest_xml_outfile2_test_.cc \
  test/gtest_xml_output_unittest_.cc \
  test/gtest_dll_test_.cc

# Python tests that we don't run.
EXTRA_DIST += \
  test/gtest_test_utils.py \
  test/gtest_xml_test_utils.py \
  test/gtest_break_on_failure_unittest.py \
  test/gtest_color_test.py \
  test/gtest_env_var_test.py \
  test/gtest_filter_unittest.py \
  test/gtest_help_test.py \
  test/gtest_list_tests_unittest.py \
  test/gtest_output_test.py \
  test/gtest_output_test_golden_lin.txt \
  test/gtest_output_test_golden_win.txt \
  test/gtest_shuffle_test.py \
  test/gtest_throw_on_failure_test.py \
  test/gtest_uninitialized_test.py \
  test/gtest_xml_outfiles_test.py \
  test/gtest_xml_output_unittest.py \
  test/run_tests_util.py \
  test/run_tests_util_test.py

# CMake script
EXTRA_DIST += \
  CMakeLists.txt

# MSVC project files
EXTRA_DIST += \
  msvc/gtest-md.sln \
  msvc/gtest.sln \
  msvc/gtest-md.vcproj \
  msvc/gtest.vcproj \
  msvc/gtest_main-md.vcproj \
  msvc/gtest_main.vcproj \
  msvc/gtest_prod_test-md.vcproj \
  msvc/gtest_prod_test.vcproj \
  msvc/gtest_unittest-md.vcproj \
  msvc/gtest_unittest.vcproj

# xcode project files
EXTRA_DIST += \
  xcode/Config/DebugProject.xcconfig \
  xcode/Config/FrameworkTarget.xcconfig \
  xcode/Config/General.xcconfig \
  xcode/Config/ReleaseProject.xcconfig \
  xcode/Config/StaticLibraryTarget.xcconfig \
  xcode/Config/TestTarget.xcconfig \
  xcode/Resources/Info.plist \
  xcode/Scripts/versiongenerate.py \
  xcode/Scripts/runtests.sh \
  xcode/gtest.xcodeproj/project.pbxproj

# xcode sample files
EXTRA_DIST += \
  xcode/Samples/FrameworkSample/Info.plist \
  xcode/Samples/FrameworkSample/runtests.sh \
  xcode/Samples/FrameworkSample/widget_test.cc \
  xcode/Samples/FrameworkSample/widget.cc \
  xcode/Samples/FrameworkSample/widget.h \
  xcode/Samples/FrameworkSample/WidgetFramework.xcodeproj/project.pbxproj

# C++Builder project files
EXTRA_DIST += \
  codegear/gtest_all.cc \
  codegear/gtest_link.cc \
  codegear/gtest.cbproj \
  codegear/gtest_main.cbproj \
  codegear/gtest_unittest.cbproj \
  codegear/gtest.groupproj

# TODO(wan@google.com): integrate scripts/gen_gtest_pred_impl.py into
#   the build system such that a user can specify the maximum predicate
#   arity here and have the script automatically generate the
#   corresponding .h and .cc files.

# Scripts and utilities
bin_SCRIPTS = scripts/gtest-config
CLEANFILES = $(bin_SCRIPTS)

# Distribute and install M4 macro
m4datadir = $(datadir)/aclocal
m4data_DATA = m4/gtest.m4
EXTRA_DIST += $(m4data_DATA)

# We define the global AM_CPPFLAGS as everything we compile includes from these
# directories.
AM_CPPFLAGS = -I$(srcdir) -I$(srcdir)/include

# Modifies compiler and linker flags for pthreads compatibility.
if HAVE_PTHREADS
  AM_CXXFLAGS = @PTHREAD_CFLAGS@ -DGTEST_HAS_PTHREAD=1
  AM_LIBS = @PTHREAD_LIBS@
else
  AM_CXXFLAGS = -DGTEST_HAS_PTHREAD=0
endif

# Build rules for libraries.
lib_LTLIBRARIES = lib/libgtest.la lib/libgtest_main.la

lib_libgtest_la_SOURCES = src/gtest-all.cc

pkginclude_HEADERS = include/gtest/gtest.h \
                     include/gtest/gtest-death-test.h \
                     include/gtest/gtest-message.h \
                     include/gtest/gtest-param-test.h \
                     include/gtest/gtest_pred_impl.h \
                     include/gtest/gtest_prod.h \
                     include/gtest/gtest-spi.h \
                     include/gtest/gtest-test-part.h \
                     include/gtest/gtest-typed-test.h

pkginclude_internaldir = $(pkgincludedir)/internal
pkginclude_internal_HEADERS = \
  include/gtest/internal/gtest-death-test-internal.h \
  include/gtest/internal/gtest-filepath.h \
  include/gtest/internal/gtest-internal.h \
  include/gtest/internal/gtest-linked_ptr.h \
  include/gtest/internal/gtest-param-util-generated.h \
  include/gtest/internal/gtest-param-util.h \
  include/gtest/internal/gtest-port.h \
  include/gtest/internal/gtest-string.h \
  include/gtest/internal/gtest-tuple.h \
  include/gtest/internal/gtest-type-util.h

lib_libgtest_main_la_SOURCES = src/gtest_main.cc
lib_libgtest_main_la_LIBADD = lib/libgtest.la

# Bulid rules for samples and tests. Automake's naming for some of
# these variables isn't terribly obvious, so this is a brief
# reference:
#
# TESTS -- Programs run automatically by "make check"
# check_PROGRAMS -- Programs built by "make check" but not necessarily run

noinst_LTLIBRARIES = samples/libsamples.la

samples_libsamples_la_SOURCES = samples/sample1.cc \
                                samples/sample1.h \
                                samples/sample2.cc \
                                samples/sample2.h \
                                samples/sample3-inl.h \
                                samples/sample4.cc \
                                samples/sample4.h

TESTS=
TESTS_ENVIRONMENT = GTEST_SOURCE_DIR="$(srcdir)/test" \
                    GTEST_BUILD_DIR="$(top_builddir)/test"
check_PROGRAMS=

# A simple sample on using gtest.
TESTS += samples/sample1_unittest
check_PROGRAMS += samples/sample1_unittest
samples_sample1_unittest_SOURCES = samples/sample1_unittest.cc
samples_sample1_unittest_LDADD = lib/libgtest_main.la \
                                 samples/libsamples.la

# Another sample.  It also verifies that libgtest works.
TESTS += samples/sample10_unittest
check_PROGRAMS += samples/sample10_unittest
samples_sample10_unittest_SOURCES = samples/sample10_unittest.cc
samples_sample10_unittest_LDADD = lib/libgtest.la

# This tests most constructs of gtest and verifies that libgtest_main
# works.
TESTS += test/gtest_all_test
check_PROGRAMS += test/gtest_all_test
test_gtest_all_test_SOURCES = test/gtest_all_test.cc
test_gtest_all_test_LDADD = lib/libgtest_main.la

# Tests that fused gtest files compile and work.
TESTS += test/gtest_fused_test
check_PROGRAMS += test/gtest_fused_test
test_gtest_fused_test_SOURCES = fused-src/gtest/gtest-all.cc \
                                fused-src/gtest/gtest_main.cc \
                                fused-src/gtest/gtest.h \
                                samples/sample1.cc samples/sample1_unittest.cc
test_gtest_fused_test_CPPFLAGS = -I"$(srcdir)/fused-src"

# Build rules for putting fused Google Test files into the distribution
# package. The user can also create those files by manually running
# scripts/fuse_gtest_files.py.
$(srcdir)/fused-src/gtest/gtest-all.cc: fused-gtest-internal

$(srcdir)/fused-src/gtest/gtest.h: fused-gtest-internal

fused-gtest-internal: $(pkginclude_HEADERS) $(pkginclude_internal_HEADERS) \
                      $(lib_libgtest_la_SOURCES) \
                      scripts/fuse_gtest_files.py
	mkdir -p "$(srcdir)/fused-src/gtest"
	chmod -R u+w "$(srcdir)/fused-src"
	rm -f "$(srcdir)/fused-src/gtest/gtest-all.cc"
	rm -f "$(srcdir)/fused-src/gtest/gtest.h"
	"$(srcdir)/scripts/fuse_gtest_files.py" "$(srcdir)/fused-src"

$(srcdir)/fused-src/gtest/gtest_main.cc: src/gtest_main.cc
	mkdir -p "$(srcdir)/fused-src/gtest"
	chmod -R u+w "$(srcdir)/fused-src"
	cp -f "$(srcdir)/src/gtest_main.cc" "$(srcdir)/fused-src/gtest"

maintainer-clean-local:
	chmod -R u+w "$(srcdir)/fused-src"
	rm -rf "$(srcdir)/fused-src/gtest"
