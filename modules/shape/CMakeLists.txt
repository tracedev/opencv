set(the_description "Shape descriptors and matchers.")

if(LIBYUV_FOUND)
       ocv_include_directories(${LIBYUV_INCLUDE_DIRS})
       link_directories(${LIBYUV_LINK_DIR})
       list(APPEND SHAPE_LIBRARIES yuv)
endif(LIBYUV_FOUND)

ocv_define_module(shape opencv_core opencv_imgproc opencv_video ${SHAPE_LIBRARIES})
