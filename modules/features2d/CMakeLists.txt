set(the_description "2D Features Framework")

if(LIBYUV_FOUND)
       ocv_include_directories(${LIBYUV_INCLUDE_DIRS})
       link_directories(${LIBYUV_LINK_DIR})
       list(APPEND FEATURES2D_LIBRARIES yuv)
endif(LIBYUV_FOUND)

ocv_define_module(features2d opencv_imgproc opencv_ml opencv_flann OPTIONAL opencv_highgui ${FEATURES2D_LIBRARIES})
