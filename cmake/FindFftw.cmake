# - Try to find FFTW
# Once done this will define
#  FFTW_FOUND - System has FFTW
#  FFTW_INCLUDE_DIRS - The FFTW include directories
#  FFTW_LIBRARIES - The libraries needed to use FFTW
#  FFTW_LINK_DIR - Path to the FFTW library

find_path(FFTW_INCLUDE_DIR fftw3.h)

find_library(FFTW_LIBRARY NAMES fftw3f)
string(REPLACE "libfftw3f.a" "" FFTW_LINK_DIR ${FFTW_LIBRARY})

set(FFTW_LIBRARIES ${FFTW_LIBRARY} )
set(FFTW_INCLUDE_DIRS ${FFTW_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set FFTW_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(Fftw  DEFAULT_MSG
            FFTW_LIBRARY FFTW_INCLUDE_DIR FFTW_LINK_DIR)

mark_as_advanced(FFTW_INCLUDE_DIR FFTW_LIBRARY FFTW_LINK_DIR)
