# FindFreetype - attempts to locate the FreeType library.
#
# This module defines the following variables (on success):
# FREETYPE_INCLUDE_DIRS - where to find freetype2/freetype.h
# FREETYPE_LIBRARIES - the libraries to link against (freetype)
# FREETYPE_FOUND - if the library was successfully located
# FREETYPE_VERSION - the version of FreeType found (if available)
#
# This module first tries to use CMake's FindPackage functionality
# to find the FreeType configuration files installed by FreeType.
# If that fails, it will attempt a more manual search.
#
# The following variables can be set to assist the search:
# FREETYPE_ROOT_DIR - root directory of a FreeType installation
#=============================================================================

# Try using the FindPackage functionality provided by FreeType itself
find_package(Freetype CONFIG QUIET)

if(FREETYPE_FOUND)
    # FreeType config found, we're done
    message(STATUS "Found FreeType via CMake config files.")
    return()
endif()

# FreeType config not found, perform a manual search

# Reset variables from the failed find_package
unset(FREETYPE_INCLUDE_DIRS)
unset(FREETYPE_LIBRARIES)
unset(FREETYPE_FOUND)
unset(FREETYPE_VERSION)

# Define preferred header search paths, prioritizing /usr/local
set(_FREETYPE_HEADER_SEARCH_DIRS
    "/usr/local/include/freetype2"
    "/usr/include/freetype2"
    "/opt/local/include/freetype2"
    "/usr/local/include"
    "/usr/include"
    "/opt/local/include"
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/includes"
    "${FREETYPE_ROOT_DIR}/include/freetype2"
    "${FREETYPE_ROOT_DIR}/include"
    NO_DEFAULT_PATH
)

# Define preferred library search paths, prioritizing /usr/local
set(_FREETYPE_LIBRARY_SEARCH_DIRS
    "/usr/local/lib"
    "/usr/lib"
    "/opt/local/lib"
    "/usr/local/lib64"
    "/usr/lib64"
    "/opt/local/lib64"
    "${FREETYPE_ROOT_DIR}/lib"
    NO_DEFAULT_PATH
)

# Locate the FreeType header
message(STATUS "Searching for freetype2/freetype.h in the following directories:")
foreach(dir ${_FREETYPE_HEADER_SEARCH_DIRS})
    message(STATUS "  ${dir}")
endforeach()

find_path(FREETYPE_INCLUDE_DIR NAMES freetype.h
    PATHS ${_FREETYPE_HEADER_SEARCH_DIRS}
    PATH_SUFFIXES freetype2/freetype
    NO_DEFAULT_PATH
)

if(NOT FREETYPE_INCLUDE_DIR)
    message(WARNING "Could not find freetype2/freetype.h. Please check the FREETYPE_ROOT_DIR or install FreeType.")
    message(STATUS "Searched directories:")
    foreach(dir ${_FREETYPE_HEADER_SEARCH_DIRS})
        message(STATUS "  ${dir}")
    endforeach()
endif()

# Locate the FreeType library
message(STATUS "Searching for FreeType library in the following directories:")
foreach(dir ${_FREETYPE_LIBRARY_SEARCH_DIRS})
    message(STATUS "  ${dir}")
endforeach()

find_library(FREETYPE_LIBRARY NAMES freetype
    PATHS ${_FREETYPE_LIBRARY_SEARCH_DIRS}
    NO_DEFAULT_PATH
)

if(NOT FREETYPE_LIBRARY)
    message(WARNING "Could not find the FreeType library. Please check the FREETYPE_ROOT_DIR or install FreeType.")
    message(STATUS "Searched directories:")
    foreach(dir ${_FREETYPE_LIBRARY_SEARCH_DIRS})
        message(STATUS "  ${dir}")
    endforeach()
endif()

# Handle standard package finding logic
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Freetype DEFAULT_MSG
    FREETYPE_INCLUDE_DIR
    FREETYPE_LIBRARY
)

if(FREETYPE_FOUND)
    set(FREETYPE_INCLUDE_DIRS "${FREETYPE_INCLUDE_DIR}")
    set(FREETYPE_LIBRARIES "${FREETYPE_LIBRARY}")

    message(STATUS "Found FreeType:")
    message(STATUS "  Include Directories: ${FREETYPE_INCLUDE_DIRS}")
    message(STATUS "  Libraries: ${FREETYPE_LIBRARIES}")
    if(FREETYPE_VERSION)
        message(STATUS "  Version: ${FREETYPE_VERSION}")
    endif()
else()
    message(STATUS "Could NOT find FreeType library.")
    if(NOT FREETYPE_INCLUDE_DIR)
        message(STATUS "  Could not find FreeType include directory.")
    endif()
    if(NOT FREETYPE_LIBRARY)
        message(STATUS "  Could not find FreeType library file.")
    endif()
endif()

mark_as_advanced(FREETYPE_INCLUDE_DIR FREETYPE_LIBRARY FREETYPE_ROOT_DIR)