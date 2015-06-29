# - Try to find LibYuv
# Once done this will define
#  LIBYUV_FOUND - System has LibYuv
#  LIBYUV_INCLUDE_DIRS - The LibYuv include directories
#  LIBYUV_LIBRARIES - The libraries needed to use LibYuv
#  LIBYUV_LINK_DIR - Path to the LibYuv library

find_path(LIBYUV_INCLUDE_DIR libyuv.h)

find_library(LIBYUV_LIBRARY NAMES yuv)
string(REPLACE "libyuv.a" "" LIBYUV_LINK_DIR ${LIBYUV_LIBRARY})

set(LIBYUV_LIBRARIES ${LIBYUV_LIBRARY} )
set(LIBYUV_INCLUDE_DIRS ${LIBYUV_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBYUV_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(LibYuv  DEFAULT_MSG
            LIBYUV_LIBRARY LIBYUV_INCLUDE_DIR LIBYUV_LINK_DIR)

mark_as_advanced(LIBYUV_INCLUDE_DIR LIBYUV_LIBRARY LIBYUV_LINK_DIR)
