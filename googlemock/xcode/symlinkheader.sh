GMOCK_INCLUDE_DIR_PATH=`pwd`/../include/gmock
mkdir -p "${CONFIGURATION_BUILD_DIR}"
cd "${CONFIGURATION_BUILD_DIR}"
rm -rf gmock
ln -fs "${GMOCK_INCLUDE_DIR_PATH}" gmock

