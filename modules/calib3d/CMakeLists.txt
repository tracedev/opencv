set(the_description "Camera Calibration and 3D Reconstruction")

if(LIBYUV_FOUND)
       ocv_include_directories(${LIBYUV_INCLUDE_DIRS})
       link_directories(${LIBYUV_LINK_DIR})
       list(APPEND CALIB3D_LIBRARIES yuv)
endif(LIBYUV_FOUND)

ocv_define_module(calib3d opencv_imgproc opencv_features2d ${CALIB3D_LIBRARIES})
