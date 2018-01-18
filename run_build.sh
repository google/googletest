#!/bin/bash
#
# Build script to generate googletest Debian package.
#
# Copyright: 2017 Ditto Technologies. All Rights Reserved.
# Author: Frankie Li, Daran He

# We need a non-root path for make install and make package.
INSTALL_DIR=${PWD}/ditto_install
mkdir -p ${INSTALL_DIR}

BUILD_DIR=build
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}
cmake .. \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
	-DBUILD_SHARED_LIBS=ON \
	-DCPACK_PACKAGING_INSTALL_PREFIX="/usr/local" \
	-DCPACK_GENERATOR="DEB" \
	-DCPACK_BINARY_DEB="ON" \
	-DCPACK_DEBIAN_PACKAGE_SHLIBDEPS="ON" \
	-DCPACK_PACKAGE_CONTACT="eng@ditto.com"

make -j$(nproc) install
