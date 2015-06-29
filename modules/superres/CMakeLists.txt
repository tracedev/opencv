if(IOS)
  ocv_module_disable(superres)
endif()

set(the_description "Super Resolution")
ocv_warnings_disable(CMAKE_CXX_FLAGS /wd4127 -Wundef -Wshadow)

if(LIBYUV_FOUND)
       ocv_include_directories(${LIBYUV_INCLUDE_DIRS})
       link_directories(${LIBYUV_LINK_DIR})
       list(APPEND SUPERRES_LIBRARIES yuv)
endif(LIBYUV_FOUND)

ocv_define_module(superres opencv_imgproc opencv_video ${SUPERRES_LIBRARIES}
                  OPTIONAL opencv_videoio opencv_cudaarithm opencv_cudafilters opencv_cudawarping opencv_cudaimgproc opencv_cudaoptflow opencv_cudacodec)
