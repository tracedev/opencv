set(the_description "Video Analysis")

if(LIBYUV_FOUND)
       ocv_include_directories(${LIBYUV_INCLUDE_DIRS})
       link_directories(${LIBYUV_LINK_DIR})
       list(APPEND VIDEO_LIBRARIES yuv)
endif(LIBYUV_FOUND)

ocv_define_module(video opencv_imgproc ${VIDEO_LIBRARIES})
