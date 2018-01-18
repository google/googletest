#!/bin/bash
# Builds debian packages using CMake and CPack.
#
# Copyright: 2017 Ditto Technologies. All Rights Reserved.
# Author: Daran He
# Reference:
#   https://dittovto.atlassian.net/wiki/spaces/DIT/pages/238845953/Jenkins+Build+Pipeline
#
# Requires:
# 	1. CMake configures CMAKE_INSTALL_PREFIX to non-root directory
#   2. CMake configures CPACK_PACKAGING_INSTALL_PREFIX to appropriate directory.
#   3. Already run `make -j$(nproc) install`
#   4. CMake version inside: DittoVersion.cmake
#   5. All Deb files exists at build/*.deb

if [ "$#" -ne 2 ]; then
    prog=`basename "$0"`
    echo "Usage: ./$prog <VERSION> <REVISION>"
    exit 1
fi

VERSION=$1
REVISION=$2
BUILD_DIR=build
CPACK_CONFIG_FILE=CPackConfig.cmake
DITTO_VERSION_FILE=DittoVersion.cmake
ARCH=`dpkg --print-architecture`
VERSION_REVISION="$VERSION-$REVISION"

cd $BUILD_DIR

# Generate CPack Configuration.
rm -f $CPACK_CONFIG_FILE
make -j$(nproc) package

# Override version information in CPack configuration.
cat ../$DITTO_VERSION_FILE >> $CPACK_CONFIG_FILE
echo "SET(CPACK_PACKAGE_VERSION \"$VERSION_REVISION\")" >> $CPACK_CONFIG_FILE

# Rebuild packages.
rm -f *.deb
cpack

# Rename all deb packages generated to ditto specifications.
for f in *.deb
do
  package_name=`dpkg-deb -I $f | grep Package: | awk '{print $NF}'`
  new_filename="${package_name}_${VERSION_REVISION}_${ARCH}.deb"
  echo "New Debian Filename: $new_filename";
  mv $f $new_filename
done
