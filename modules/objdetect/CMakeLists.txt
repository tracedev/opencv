set(the_description "Object Detection")

if(LIBYUV_FOUND)
       ocv_include_directories(${LIBYUV_INCLUDE_DIRS})
       link_directories(${LIBYUV_LINK_DIR})
       list(APPEND OBJDETECT_LIBRARIES yuv)
endif(LIBYUV_FOUND)

ocv_define_module(objdetect opencv_core opencv_imgproc opencv_ml OPTIONAL opencv_highgui ${OBJDETECT_LIBRARIES})
