# - Try to find TracerCommon
# Once done this will define
#  TRACERCOMMON_FOUND - System has TracerCommon
#  TRACERCOMMON_INCLUDE_DIRS - The TraccerCommon include directories
#  TRACERCOMMON_LIBRARIES - The libraries needed to use TracerCommon
#  TRACERCOMMON_LINK_DIR - Path to the TracerCommon library

find_path(TRACERCOMMON_INCLUDE_DIR tracercommon.h)

find_library(TRACERCOMMON_LIBRARY NAMES tracercommon)
string(REPLACE "libtracercommon.a" "" TRACERCOMMON_LINK_DIR ${TRACERCOMMON_LIBRARY})

set(TRACERCOMMON_LIBRARIES ${TRACERCOMMON_LIBRARY} )
set(TRACERCOMMON_INCLUDE_DIRS ${TRACERCOMMON_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set TRACERCOMMON_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(TracerCommon  DEFAULT_MSG
            TRACERCOMMON_LIBRARY TRACERCOMMON_INCLUDE_DIR TRACERCOMMON_LINK_DIR)

mark_as_advanced(TRACERCOMMON_INCLUDE_DIR TRACERCOMMON_LIBRARY TRACERCOMMON_LINK_DIR)
