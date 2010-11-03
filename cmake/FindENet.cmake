# - Find ENet
# Find the ENet libraries (asound)
#
#  This module defines the following variables:
#     ENET_FOUND           - True if ENET_INCLUDE_DIR & ENET_LIBRARY
#                            are found
#     ENET_LIBRARIES       - Set when ENET_LIBRARY is found
#     ENET_INCLUDE_DIRS    - Set when ENET_INCLUDE_DIR is found
#
#     ENET_INCLUDE_DIR     - where to find enet.h, etc.
#     ENET_LIBRARY         - the enet library
#

#=============================================================================
#=============================================================================

find_path(ENET_INCLUDE_DIR NAMES enet.h
          PATH_SUFFIXES enet
          DOC "ENet include directory"
)

find_library(ENET_LIBRARY NAMES enet
          DOC "The ENet library"
)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ENET DEFAULT_MSG ENET_LIBRARY ENET_INCLUDE_DIR)

if(ENET_FOUND)
  set( ENET_LIBRARIES ${ENET_LIBRARY} )
  set( ENET_INCLUDE_DIRS ${ENET_INCLUDE_DIR} )
endif()

mark_as_advanced(ENET_INCLUDE_DIR ENET_LIBRARY)
