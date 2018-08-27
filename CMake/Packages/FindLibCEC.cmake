# - Try to find libCEC
# Once done, this will define
#
#  LIBCEC_FOUND - system has libCEC
#  LIBCEC_INCLUDE_DIRS - the libCEC include directories 
#  LIBCEC_LIBRARIES - link these to use libCEC

include(FindPkgMacros)
findpkg_begin(LIBCEC)

# Get path, convert backslashes as ${ENV_${var}}
getenv_path(LIBCEC_HOME)

# construct search paths
set(LIBCEC_PREFIX_PATH ${LIBCEC_HOME} ${ENV_LIBCEC_HOME})
create_search_paths(LIBCEC)

# redo search if prefix path changed
clear_if_changed(
  LIBCEC_PREFIX_PATH
  LIBCEC_LIBRARY_REL
  LIBCEC_INCLUDE_DIR
)

set(LIBCEC_LIBRARY_NAMES libcec.a libcec.dylib)

use_pkgconfig(LIBCEC_PKGC libcec)

findpkg_framework(libCEC)

find_path(LIBCEC_INCLUDE_DIR NAMES cec.h HINTS ${LIBCEC_INC_SEARCH_PATH} ${LIBCEC_PKGC_INCLUDE_DIRS})

find_library(LIBCEC_LIBRARY_REL NAMES ${LIBCEC_LIBRARY_NAMES} HINTS ${LIBCEC_LIB_SEARCH_PATH} ${LIBCEC_PKGC_LIBDIR})

make_library_set(LIBCEC_LIBRARY)

findpkg_finish(LIBCEC)
