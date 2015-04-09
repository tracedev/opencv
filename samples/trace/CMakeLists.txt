# ----------------------------------------------------------------------------
#  CMake file for trace samples. See root CMakeLists.txt
#
# ----------------------------------------------------------------------------

SET(OPENCV_CPP_SAMPLES_REQUIRED_DEPS opencv_core opencv_imgproc opencv_photo opencv_video opencv_features2d 
    opencv_flann opencv_objdetect opencv_calib3d opencv_imgcodecs opencv_videoio opencv_highgui opencv_ml)

ocv_check_dependencies(${OPENCV_TRACE_SAMPLES_REQUIRED_DEPS})


if(BUILD_EXAMPLES AND OCV_DEPENDENCIES_FOUND)
  project(trace_samples)

  ocv_include_directories("${OpenCV_SOURCE_DIR}/include")#for opencv.hpp
  ocv_include_modules(${OPENCV_CPP_SAMPLES_REQUIRED_DEPS})

  if(HAVE_opencv_cudaoptflow)
    ocv_include_directories("${OpenCV_SOURCE_DIR}/modules/cudaoptflow/include")
  endif()
  if(HAVE_opencv_cudaimgproc)
    ocv_include_directories("${OpenCV_SOURCE_DIR}/modules/cudaimgproc/include")
  endif()
  if(HAVE_opencv_cudaarithm)
    ocv_include_directories("${OpenCV_SOURCE_DIR}/modules/cudaarithm/include")
  endif()
  if(HAVE_opencv_cudafilters)
    ocv_include_directories("${OpenCV_SOURCE_DIR}/modules/cudafilters/include")
  endif()

  if (HAVE_opencv_xfeatures2d)
    ocv_include_directories("${OPENCV_MODULE_opencv_xfeatures2d_LOCATION}/include")
  endif()

  if(HAVE_opencv_ocl)
    ocv_include_directories("${OpenCV_SOURCE_DIR}/modules/ocl/include")
  endif()

  if(CMAKE_COMPILER_IS_GNUCXX AND NOT ENABLE_NOISY_WARNINGS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-function")
  endif()

  # ---------------------------------------------
  #      Define executable targets
  # ---------------------------------------------
  MACRO(OPENCV_DEFINE_TRACE_EXAMPLE name srcs)

    set(sample_kind example)
    set(sample_KIND EXAMPLE)
    set(sample_subfolder "trace")

    set(the_target "${sample_kind}_${name}")
    add_executable(${the_target} ${srcs})
    ocv_target_link_libraries(${the_target} ${OPENCV_LINKER_LIBS} ${OPENCV_CPP_SAMPLES_REQUIRED_DEPS})

    if(HAVE_opencv_ocl)
      ocv_target_link_libraries(${the_target} opencv_ocl)
    endif()

    set_target_properties(${the_target} PROPERTIES
      OUTPUT_NAME "trace-${sample_kind}-${name}"
      PROJECT_LABEL "(${sample_KIND}) ${name}")

    if(ENABLE_SOLUTION_FOLDERS)
      set_target_properties(${the_target} PROPERTIES FOLDER "samples/${sample_subfolder}")
    endif()

  ENDMACRO()

  file(GLOB_RECURSE trace_samples RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

  # Filter out non color motion tracking for now and use gray only
  ocv_list_filterout(trace_samples "motiontracknoncolor")

  foreach(sample_filename ${trace_samples})
    get_filename_component(sample ${sample_filename} NAME_WE)
    OPENCV_DEFINE_TRACE_EXAMPLE(${sample}  ${sample_filename})
  endforeach()

  #include("tutorial_code/calib3d/real_time_pose_estimation/CMakeLists.txt")
endif()

if(INSTALL_C_EXAMPLES AND NOT WIN32)
  file(GLOB C_SAMPLES *.c *.cpp *.jpg *.png *.data makefile.* build_all.sh *.dsp *.cmd )
  install(FILES ${C_SAMPLES}
          DESTINATION ${OPENCV_SAMPLES_SRC_INSTALL_PATH}/trace
          PERMISSIONS OWNER_READ GROUP_READ WORLD_READ COMPONENT samples)
endif()
