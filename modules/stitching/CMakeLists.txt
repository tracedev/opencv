set(the_description "Images stitching")

if(LIBYUV_FOUND)
       ocv_include_directories(${LIBYUV_INCLUDE_DIRS})
       link_directories(${LIBYUV_LINK_DIR})
       list(APPEND STITCHING_LIBRARIES yuv)
endif(LIBYUV_FOUND)

ocv_define_module(stitching opencv_imgproc opencv_features2d opencv_calib3d opencv_objdetect ${STITCHING_LIBRARIES}
                  OPTIONAL opencv_cuda opencv_cudaarithm opencv_cudafilters opencv_cudafeatures2d opencv_xfeatures2d)
