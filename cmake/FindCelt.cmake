# - Find Celt
# Find the Celt libraries (asound)
#
#  This module defines the following variables:
#     CELT_FOUND           - True if CELT_INCLUDE_DIR & CELT_LIBRARY
#                            are found
#     CELT_LIBRARIES       - Set when CELT_LIBRARY is found
#     CELT_INCLUDE_DIRS    - Set when CELT_INCLUDE_DIR is found
#
#     CELT_INCLUDE_DIR     - where to find celt.h, etc.
#     CELT_LIBRARY         - the celt library
#

#=============================================================================
#=============================================================================

find_path(CELT_INCLUDE_DIR NAMES celt.h
          PATH_SUFFIXES celt
          DOC "Celt include directory"
)

find_library(CELT_LIBRARY NAMES celt0 celt
          DOC "The Celt library"
)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CELT DEFAULT_MSG CELT_LIBRARY CELT_INCLUDE_DIR)

if(CELT_FOUND)
  set( CELT_LIBRARIES ${CELT_LIBRARY} )
  set( CELT_INCLUDE_DIRS ${CELT_INCLUDE_DIR} )
endif()

mark_as_advanced(CELT_INCLUDE_DIR CELT_LIBRARY)
