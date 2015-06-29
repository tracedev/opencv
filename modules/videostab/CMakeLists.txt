set(the_description "Video stabilization")

if(LIBYUV_FOUND)
       ocv_include_directories(${LIBYUV_INCLUDE_DIRS})
       link_directories(${LIBYUV_LINK_DIR})
       list(APPEND VIDEOSTAB_LIBRARIES yuv)
endif(LIBYUV_FOUND)

ocv_define_module(videostab opencv_imgproc opencv_features2d opencv_video opencv_photo opencv_calib3d ${VIDEOSTAB_LIBRARIES}
                  OPTIONAL opencv_cuda opencv_cudawarping opencv_cudaoptflow opencv_videoio)
