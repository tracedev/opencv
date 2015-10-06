set(the_description "Image Processing")

if(LIBYUV_FOUND)
       ocv_include_directories(${LIBYUV_INCLUDE_DIRS})
       link_directories(${LIBYUV_LINK_DIR})
       list(APPEND IMGPROC_LIBRARIES yuv)
endif(LIBYUV_FOUND)

if(TRACERCOMMON_FOUND)
       ocv_include_directories(${TRACERCOMMON_INCLUDE_DIRS})
       link_directories(${TRACERCOMMON_LINK_DIR})
       list(APPEND IMGPROC_LIBRARIES tracercommon)
endif(TRACERCOMMON_FOUND)

ocv_define_module(imgproc opencv_core ${IMGPROC_LIBRARIES})
