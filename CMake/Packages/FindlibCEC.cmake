# - Try to find libCEC
# Once done, this will define
#
#  libCEC_FOUND - system has libCEC
#  libCEC_INCLUDE_DIRS - the libCEC include directories 
#  libCEC_LIBRARIES - link these to use libCEC

include(FindPkgMacros)
findpkg_begin(libCEC)

# Get path, convert backslashes as ${ENV_${var}}
getenv_path(LIBCEC_HOME)

# construct search paths
set(libCEC_PREFIX_PATH ${LIBCEC_HOME} ${ENV_LIBCEC_HOME})
create_search_paths(LIBCEC)
# redo search if prefix path changed
clear_if_changed(libCEC_PREFIX_PATH
  libCEC_LIBRARY_FWK
  libCEC_LIBRARY_REL
  libCEC_LIBRARY_DBG
  libCEC_INCLUDE_DIR
)

set(libCEC_LIBRARY_NAMES libcec.so)
get_debug_names(libCEC_LIBRARY_NAMES)

use_pkgconfig(libCEC_PKGC libcec)

findpkg_framework(libCEC)

find_path(libCEC_INCLUDE_DIR NAMES cec.h HINTS ${libCEC_INC_SEARCH_PATH} ${libCEC_PKGC_INCLUDE_DIRS})

find_library(libCEC_LIBRARY_REL NAMES ${libCEC_LIBRARY_NAMES} HINTS ${libCEC_LIB_SEARCH_PATH} ${libCEC_PKGC_LIBRARY_DIRS} PATH_SUFFIXES release relwithdebinfo minsizerel)
find_library(libCEC_LIBRARY_DBG NAMES ${libCEC_LIBRARY_NAMES_DBG} HINTS ${libCEC_LIB_SEARCH_PATH} ${libCEC_PKGC_LIBRARY_DIRS} PATH_SUFFIXES debug)

make_library_set(libCEC_LIBRARY)

findpkg_finish(libCEC)