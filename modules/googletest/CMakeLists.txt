zephyr_library()

# Disable use of pthreads in GoogleTest
set(gtest_disable_pthreads ON CACHE BOOL "Build GoogleTest without Pthread")

zephyr_include_directories_ifdef(CONFIG_GTEST ${ZEPHYR_GOOGLETEST_MODULE_DIR}/googletest/)
zephyr_include_directories_ifdef(CONFIG_GTEST ${ZEPHYR_GOOGLETEST_MODULE_DIR}/googletest/include/)
zephyr_library_sources_ifdef(CONFIG_GTEST
    ${ZEPHYR_GOOGLETEST_MODULE_DIR}/googletest/src/gtest-all.cc
)
target_sources_ifdef(CONFIG_GTEST app PRIVATE
    ${ZEPHYR_GOOGLETEST_MODULE_DIR}/googletest/src/gtest_main.cc
)

zephyr_include_directories_ifdef(CONFIG_GMOCK ${ZEPHYR_GOOGLETEST_MODULE_DIR}/googlemock/)
zephyr_include_directories_ifdef(CONFIG_GMOCK ${ZEPHYR_GOOGLETEST_MODULE_DIR}/googlemock/include/)
zephyr_library_sources_ifdef(CONFIG_GMOCK ${ZEPHYR_GOOGLETEST_MODULE_DIR}/googlemock/src/gmock-all.cc)
