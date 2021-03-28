# - Find Dwarf
# Find the dwarf.h header from elf utils
#
#  DWARF_INCLUDE_DIR - where to find dwarf.h, etc.
#  DWARF_LIBRARIES   - List of libraries when using elf utils.
#  DWARF_FOUND       - True if fdo found.

message(STATUS "Checking availability of DWARF and ELF development libraries")

INCLUDE(CheckLibraryExists)

if (DWARF_INCLUDE_DIR AND ELF_INCLUDE_DIR
    AND LIBDW_INCLUDE_DIR AND DWARF_LIBRARY AND ELF_LIBRARY)
    # Already in cache, be silent
    set(DWARF_FIND_QUIETLY TRUE)
endif (DWARF_INCLUDE_DIR AND ELF_INCLUDE_DIR
       AND LIBDW_INCLUDE_DIR AND DWARF_LIBRARY AND ELF_LIBRARY)

if (DEFINED LIBDW_PATH)
    set (USER_LIBDW_INCLUDE_PATH ${LIBDW_PATH}/include)
    set (USER_LIBDW_LIB_PATH ${LIBDW_PATH})
else ()
    set (USER_LIBDW_INCLUDE_PATH "")
    set (USER_LIBDW_LIB_PATH "")
endif ()

if (DEFINED LIBELF_PATH)
    set (USER_LIBELF_INCLUDE_PATH ${LIBELF_PATH}/include)
    set (USER_LIBELF_LIB_PATH ${LIBELF_PATH})
else ()
    set (USER_LIBELF_INCLUDE_PATH "")
    set (USER_LIBELF_LIB_PATH "")
endif ()

find_path(DWARF_INCLUDE_DIR dwarf.h
    HINTS ${USER_LIBDW_INCLUDE_PATH}
)

find_path(ELF_INCLUDE_DIR elf.h
    HINTS ${USER_LIBELF_INCLUDE_PATH}
)

find_path(LIBDW_INCLUDE_DIR elfutils/libdw.h
    HINTS ${USER_LIBDW_INCLUDE_PATH}
)

find_library(DWARF_LIBRARY
    NAMES dw
    HINTS ${USER_LIBDW_LIB_PATH}
    PATH_SUFFIXES lib lib64
)

find_library(ELF_LIBRARY
    NAMES elf
    HINTS ${USER_LIBELF_LIB_PATH}
    PATH_SUFFIXES lib lib64
)

if (DWARF_INCLUDE_DIR AND ELF_INCLUDE_DIR AND LIBDW_INCLUDE_DIR AND DWARF_LIBRARY AND ELF_LIBRARY)
    set(DWARF_FOUND TRUE)
    set(DWARF_LIBRARIES ${DWARF_LIBRARY} ${ELF_LIBRARY})

    set(CMAKE_REQUIRED_LIBRARIES ${DWARF_LIBRARIES})
    # check if libdw have the dwfl_module_build_id routine,
    # i.e. if it supports the buildid mechanism to match binaries
    # to detached debug info sections (the -debuginfo packages in
    # distributions such as fedora). We do it against libelf
    # because, IIRC, some distros include libdw linked statically
    # into libelf.
    check_library_exists(elf dwfl_module_build_id "" HAVE_DWFL_MODULE_BUILD_ID)
else (DWARF_INCLUDE_DIR AND ELF_INCLUDE_DIR
      AND LIBDW_INCLUDE_DIR AND DWARF_LIBRARY AND ELF_LIBRARY)
    set(DWARF_FOUND FALSE)
    set(DWARF_LIBRARIES)
endif (DWARF_INCLUDE_DIR AND ELF_INCLUDE_DIR
       AND LIBDW_INCLUDE_DIR AND DWARF_LIBRARY AND ELF_LIBRARY)

if (DWARF_FOUND)
    if (NOT DWARF_FIND_QUIETLY)
        message(STATUS "Found dwarf.h header: ${DWARF_INCLUDE_DIR}")
        message(STATUS "Found elf.h header: ${ELF_INCLUDE_DIR}")
        message(STATUS "Found elfutils/libdw.h header: ${LIBDW_INCLUDE_DIR}")
        message(STATUS "Found libdw library: ${DWARF_LIBRARY}")
        message(STATUS "Found libelf library: ${ELF_LIBRARY}")
    endif (NOT DWARF_FIND_QUIETLY)
else (DWARF_FOUND)
    if (DWARF_FIND_REQUIRED)
        # Check if we are in a Red Hat (RHEL) or Fedora system to tell
        # exactly which packages should be installed. Please send
        # patches for other distributions.
        find_path(FEDORA fedora-release /etc)
        find_path(REDHAT redhat-release /etc)
        if (FEDORA OR REDHAT)
            if (NOT DWARF_INCLUDE_DIR OR NOT ELF_INCLUDE_DIR
                OR NOT LIBDW_INCLUDE_DIR)
                message(STATUS "Please install the elfutils-devel package")
            endif (NOT DWARF_INCLUDE_DIR OR NOT ELF_INCLUDE_DIR
                   OR NOT LIBDW_INCLUDE_DIR)
            if (NOT DWARF_LIBRARY)
                message(STATUS "Please install the elfutils-libs package")
            endif (NOT DWARF_LIBRARY)
            if (NOT ELF_LIBRARY)
                message(STATUS "Please install the elfutils-libelf package")
            endif (NOT ELF_LIBRARY)
        else (FEDORA OR REDHAT)
            if (NOT DWARF_INCLUDE_DIR)
                message(STATUS "Could NOT find dwarf include dir")
            endif (NOT DWARF_INCLUDE_DIR)
            if (NOT ELF_INCLUDE_DIR)
                message(STATUS "Could NOT find elf include dir")
            endif (NOT ELF_INCLUDE_DIR)
            if (NOT LIBDW_INCLUDE_DIR)
                message(STATUS "Could NOT find libdw include dir")
            endif (NOT LIBDW_INCLUDE_DIR)
            if (NOT DWARF_LIBRARY)
                message(STATUS "Could NOT find libdw library")
            endif (NOT DWARF_LIBRARY)
            if (NOT ELF_LIBRARY)
                message(STATUS "Could NOT find libelf library")
            endif (NOT ELF_LIBRARY)
        endif (FEDORA OR REDHAT)
        message(FATAL_ERROR "Could NOT find some ELF and DWARF libraries, please install the missing packages")
    endif (DWARF_FIND_REQUIRED)
endif (DWARF_FOUND)

mark_as_advanced(DWARF_INCLUDE_DIR ELF_INCLUDE_DIR LIBDW_INCLUDE_DIR DWARF_LIBRARY ELF_LIBRARY)

message(STATUS "Checking availability of DWARF and ELF development libraries - done")
