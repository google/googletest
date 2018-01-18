#!/bin/bash
# Get Ditto Debian Package Version in X.Y.Z format.
#
# Copyright: 2017 Ditto Technologies. All Rights Reserved.
# Author: Daran He

CPACK_CONFIG_FILE=DittoVersion.cmake

get_cmake_var() {
	VAR_NAME=$1
	echo `grep $VAR_NAME $CPACK_CONFIG_FILE | tail -1 | cut -d '"' -f2`
}

VERSION_MAJOR=$(get_cmake_var CPACK_PACKAGE_VERSION_MAJOR)
VERSION_MINOR=$(get_cmake_var CPACK_PACKAGE_VERSION_MINOR)
VERSION_PATCH=$(get_cmake_var CPACK_PACKAGE_VERSION_PATCH)
VERSION="$VERSION_MAJOR.$VERSION_MINOR.$VERSION_PATCH"
echo $VERSION
