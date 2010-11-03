# - Find Jack
# Find the Jack libraries (asound)
#
#  This module defines the following variables:
#     JACK_FOUND           - True if JACK_INCLUDE_DIR & JACK_LIBRARY
#                            are found
#     JACK_LIBRARIES       - Set when JACK_LIBRARY is found
#     JACK_INCLUDE_DIRS    - Set when JACK_INCLUDE_DIR is found
#
#     JACK_INCLUDE_DIR     - where to find jack.h, etc.
#     JACK_LIBRARY         - the jack library
#

#=============================================================================
#=============================================================================

find_path(JACK_INCLUDE_DIR NAMES jack.h
          PATH_SUFFIXES jack
          DOC "Jack include directory"
)

find_library(JACK_LIBRARY NAMES jack
          DOC "The Jack library"
)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(JACK DEFAULT_MSG JACK_LIBRARY JACK_INCLUDE_DIR)

if(JACK_FOUND)
  set( JACK_LIBRARIES ${JACK_LIBRARY} )
  set( JACK_INCLUDE_DIRS ${JACK_INCLUDE_DIR} )
endif()

mark_as_advanced(JACK_INCLUDE_DIR JACK_LIBRARY)
