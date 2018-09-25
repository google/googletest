## Process this file with automake to produce Makefile.in
AM_CXXFLAGS = -std=gnu++11

ACLOCAL_AMFLAGS = -I m4

AUTOMAKE_OPTIONS = foreign

# Build . before src so that our all-local and clean-local hooks kicks in at
# the right time.
SUBDIRS = googletest googlemock

EXTRA_DIST = \
  BUILD.bazel \
  CMakeLists.txt \
  README.md \
  WORKSPACE
