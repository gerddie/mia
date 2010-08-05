
OPTION(WITH_OpenEXR "enable support for OpenEXR" ${SEARCH_LIBS_DEFAULT})

IF(WITH_OpenEXR)
  
  if (STRICT_DEPENDECIES)
    pkg_check_modules(EXR REQUIRED OpenEXR)
  else (STRICT_DEPENDECIES)
    pkg_check_modules(EXR OpenEXR)
  endif (STRICT_DEPENDECIES)
  
  
  IF( EXR_LIBRARIES AND EXR_INCLUDE_DIRS )
    
    SET(image2dio_path ${PLUGIN_INSTALL_PATH}/io/2dimage)
    SET(image3dio_path ${PLUGIN_INSTALL_PATH}/io/3dimage)
    SET(vf2dio_path ${PLUGIN_INSTALL_PATH}/io/2dvf)
    
    SET(EXR_LINK_LIBS ${MIA2DLIBS} ${EXR_LIBRARIES})

    LINK_DIRECTORIES(${EXR_LIBRARY_DIRS})

    SET(DEPS2D mia2d ${EXR_LIBRARIES})
    
    INCLUDE_DIRECTORIES(${EXR_INCLUDE_DIRS})
    
    PLUGIN(2dimgexr    "${DEPS2D}" "${image2dio_path}")
    PLUGIN(2dvfexr     "${DEPS2D}" "${vf2dio_path}")
    
    # test programs 
    ADD_EXECUTABLE(test-openexr test_openexr.cc)
    ADD_DEPENDENCIES(test-openexr 2dvfexr 2dimgexr)
    
    SET(DEPLIBS mia2dtest mia2d ${BASELIBS})
    TARGET_LINK_LIBRARIES(test-openexr ${DEPLIBS} ${BOOST_UNITTEST})
    
    IF(NOT WIN32)
      #a wrapper script is needed, to add the current build dir to the search path for dlopen
      ADD_TEST(exr sh ${CMAKE_CURRENT_SOURCE_DIR}/test-openexr.sh )
    ENDIF(NOT WIN32)  
    
  ENDIF(EXR_LIBRARIES AND EXR_INCLUDE_DIRS)
  
ENDIF(WITH_OpenEXR)
