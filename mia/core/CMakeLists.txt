#
# The miacore build definitions
# Copyright (c) Leipzig, Madrid 2004-2010
# Redistribution according to the GPL 
#

SET(MIACORE_SRC_BASE 
  attributes.cc 
  boundary_conditions.cc
  callback.cc 
  cost.cc 
  combiner.cc
  cmdlineparser.cc
  creator.cc
  datapool.cc
  dlloader.cc
  dummyhandler.cc
  fatcost.cc
  file.cc 
  filetools.cc 
  filter.cc
  flagstring.cc
  fullstats.cc
  history.cc
  index.cc
  info.cc
  splinekernel.cc
  interpolator1d.cc
  iodata.cc
  ioplugin.cc
  labelmap.cc
  mitestimages.cc
  module.cc
  msgstream.cc 
  minimizer.cc
  noisegen.cc 
  optionparser.cc 
  optparam.cc 
  parameter.cc  
  pixeltype.cc 
  plugin_base.cc  
  product_base.cc
  property_flags.cc
  probmap.cc
  regmodel.cc
  revision.cc 
  scaler1d.cc
  seriesstats.cc 
  shape.cc 
  slopestatistics.cc
  slopeclassifier.cc
  spacial_kernel.cc 
  splineparzenmi.cc
  sqmin.cc
  streamredir.cc
  testplugin.cc 
  threadedmsg.cc
  typedescr.cc
  utils.cc 
  watch.cc 
  )

SET(MIACORE_HEADER_BASE
  attributes.hh
  boundary_conditions.hh
  callback.hh
  cmdlineparser.hh
  cost.hh
  cost.cxx
  combiner.hh
  creator.hh
  datapool.hh
  defines.hh
  delayedparameter.hh
  dictmap.hh
  dlloader.hh
  dummyhandler.hh
  errormacro.hh
  export_handler.hh
  factory.hh
  fatcost.hh fatcost.cxx
  file.hh
  filetools.hh
  filter.hh
  flagstring.hh
  fullstats.hh
  handler.cxx handler.hh
  history.hh
  histogram.hh
  index.hh
  splinekernel.hh
  import_handler.hh
  iodata.hh
  iohandler.cxx iohandler.hh
  ioplugin.cxx ioplugin.hh
  labelmap.hh
  mitestimages.hh
  module.hh
  msgstream.hh
  minimizer.hh
  noisegen.hh
  optionparser.hh
  optparam.hh
  parameter.cxx parameter.hh
  pixeltype.hh
  plugin_base.cxx plugin_base.hh
  probmap.hh
  property_flags.hh
  product_base.hh
  regmodel.hh
  scaler1d.hh
  shape.hh shape.cxx
  seriesstats.hh
  slopestatistics.hh
  slopeclassifier.hh
  splineparzenmi.hh
  spacial_kernel.hh
  sqmin.hh
  streamredir.hh
  typedescr.hh
  traits.hh
  testplugin.hh
  threadedmsg.hh
  watch.hh
  )

SET(MIACORETEST_SRC 
  test_core.cc
  test_cost.cc
  test_filter.cc
  test_handler.cc
  test_kernels.cc
  test_optparam.cc
  test_optionparser.cc
  test_parameter.cc
  test_pixeltype.cc
  test_register.cc
  test_sqmin.cc
  test_streamredir.cc
  test_dictmap.cc
  test_fifofilter.cc
  test_probmap.cc
)

IF(ITPP_FOUND)
  SET(ITPP_SRC 
    ica.cc 
    )
  SET(ITPP_HEADER 
    ica.hh
    )
ENDIF(ITPP_FOUND)

IF(FFTWF_FOUND AND WITH_FFTWF)
  SET(FFTWF_SRC 
    fft1d_r2c.cc 
    )
  SET(FFTWF_HEADER 
    fft1d_r2c.hh
    )
ENDIF(FFTWF_FOUND AND WITH_FFTWF)

IF(PWPDF_FOUND) 
  SET(PWPDF_SRC 
    pwh.cc
    )
  SET(PWPDF_HEADER 
    pwh.hh
    )
ENDIF(PWPDF_FOUND) 

MACRO(CORE_TEST name)
  ADD_EXECUTABLE(test-${name} test_${name}.cc)  
  TARGET_LINK_LIBRARIES(test-${name} ${MIACORE} ${BOOST_UNITTEST})
  ADD_TEST(core-${name} test-${name})
  IF(WIN32)
    SET_TARGET_PROPERTIES(test-${name} PROPERTIES LINKER_FLAGS "/NODEFAULTLIB:MSVCRT")
  ENDIF(WIN32)
ENDMACRO(CORE_TEST name)

#
# create the revision retrival code
#

IF (UNIX)
# probably should look for bash first 
  ADD_CUSTOM_TARGET(revision.hh COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/revision.sh ${CMAKE_SOURCE_DIR})
ELSEIF (WIN32)
  ADD_CUSTOM_TARGET(revision.hh COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/revision.bat ${CMAKE_SOURCE_DIR})
ELSE (UNIX)
  MESSAGE(FATAL_ERROR "Sorry, don't know how to create revision.hh in the build root directory using your OS") 
ENDIF (UNIX)



#
# the library target 
#
SET(MIACORE_SRC ${MIACORE_SRC_BASE} ${ITPP_SRC} ${FFTWF_SRC} ${PWPDF_SRC})
SET(MIACORE_HEADER ${MIACORE_HEADER_BASE} ${ITPP_HEADER} ${FFTWF_HEADER} ${PWPDF_HEADER})

ADD_LIBRARY(miacore SHARED ${MIACORE_SRC})
ADD_DEPENDENCIES(miacore revision.hh)
TARGET_LINK_LIBRARIES(miacore miagsl ${BASELIBS} ${TBB_LIBRARIES})


IF(XMLPP_FOUND)
  TARGET_LINK_LIBRARIES(miacore ${XMLPP_LIBRARIES})
ENDIF(XMLPP_FOUND)


IF(FFTWF_FOUND AND WITH_FFTWF)
  TARGET_LINK_LIBRARIES(miacore ${FFTWF_LIB})
ENDIF(FFTWF_FOUND AND WITH_FFTWF)

IF(PWPDF_FOUND AND FFTWD_FOUND)
  TARGET_LINK_LIBRARIES(miacore ${PWPDF_LIBRARIES})
ENDIF(PWPDF_FOUND AND FFTWD_FOUND)

IF(ITPP_FOUND)
  TARGET_LINK_LIBRARIES(miacore ${ITPP_LIBRARIES})
ENDIF(ITPP_FOUND)

SET(MIACORE miacore ${BASELIBS})
SET_TARGET_PROPERTIES(miacore PROPERTIES 
  VERSION ${LIBRARY_VERSION_INFO}
)
#
# the test targets 
#

ADD_EXECUTABLE(test-core ${MIACORETEST_SRC})
ADD_TEST(core test-core)

TARGET_LINK_LIBRARIES(test-core ${MIACORE} ${BOOST_UNITTEST})

IF(WIN32)
  SET_TARGET_PROPERTIES(test-core PROPERTIES LINKER_FLAGS "/NODEFAULTLIB:MSVCRT")
ENDIF(WIN32)

IF (NOT WIN32)
  CORE_TEST(history)
ENDIF (NOT WIN32)
#CORE_TEST(noisegen)

DEFEXE(plugin_test miacore)

ADD_SUBDIRECTORY(testplug      )
ADD_SUBDIRECTORY(spacialkernel )
ADD_SUBDIRECTORY(noise         ) 
ADD_SUBDIRECTORY(minimizer     ) 
ADD_SUBDIRECTORY(splinekernel  ) 
ADD_SUBDIRECTORY(splinebc  ) 

NEW_TEST(attributes miacore)
NEW_TEST(boundary_conditions  miacore)
NEW_TEST(callback miacore)
NEW_TEST(cmdlineparser miacore)
NEW_TEST(datapool miacore)
NEW_TEST(delayedparameter miacore)
NEW_TEST(factoryoption miacore)
NEW_TEST(fatcost  miacore)
NEW_TEST(filetools miacore)
NEW_TEST(flagstring  miacore)
NEW_TEST(fullstats miacore)
NEW_TEST(histogram miacore)
NEW_TEST(index miacore)
NEW_TEST(iohandler miacore)
NEW_TEST(interpolator1d miacore)
NEW_TEST(kmeans miacore)
NEW_TEST(labelmap miacore)
NEW_TEST(meanvar  miacore)
NEW_TEST(minimizer miacore)
NEW_TEST(property_flags  miacore)
NEW_TEST(scaler1d miacore)
NEW_TEST(slopestatistics miacore)
NEW_TEST(slopeclassifier miacore)
NEW_TEST(statistics miacore)
NEW_TEST(splineparzenmi miacore)
NEW_TEST(seriesstats miacore)
NEW_TEST(simpson miacore)
NEW_TEST(splinekernel miacore)
NEW_TEST(threadedmsg miacore)
NEW_TEST(tools miacore)
NEW_TEST(utils miacore)
NEW_TEST(Vector miacore)

IF(ITPP_FOUND)
  NEW_TEST(ica miacore)
ENDIF(ITPP_FOUND)

IF(FFTWF_FOUND AND WITH_FFTWF)
  NEW_TEST(fft1d miacore)
ENDIF(FFTWF_FOUND AND WITH_FFTWF)

IF(PWPDF_FOUND) 
  NEW_TEST(pwh miacore)
ENDIF(PWPDF_FOUND) 
#
#installation 
#

INSTALL_WITH_EXPORT(miacore)
INSTALL(FILES ${MIACORE_HEADER} DESTINATION ${INCLUDE_INSTALL_PATH}/mia/core)
