#
# This file is part of MIA - a toolbox for medical image analysis 
# Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
#
# MIA is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
#

SET(GSLPP_SOURCE 
  gsl_matrix.cc
  gsl_matrix_vector_ops.cc
  gsl_multimin.cc
  gsl_pca.cc 
  gsl_vector.cc
  gsl_wavelet.cc
  )

SET(GSLPP_HEADERS 
  gsl_matrix.hh
  gsl_matrix_vector_ops.hh
  gsl_multimin.hh
  gsl_pca.hh
  gsl_vector.hh
  gsl_wavelet.hh
  gsl_defines.hh
  )

SET(MIACORE_SRC_BASE
  ${GSLPP_SOURCE} 
  attributes.cc 
  attribute_names.cc 
  boundary_conditions.cc
  callback.cc 
  cost.cc 
  combiner.cc
  convergence_measure.cc
  cmdlineparser.cc
  cmdoption.cc
  cmdstringoption.cc
  cmdbooloption.cc
  cmeans.cc
  creator.cc
  datapool.cc
  distance.cc
  dlloader.cc
  dummyhandler.cc
  errormacro.cc
  fastica.cc
  fastica_nonlinearity.cc
  file.cc 
  filetools.cc 
  filter.cc
  fixedwidthoutput.cc
  fft1d_r2c.cc 
  fftslopeclassifier.cc
  flagstring.cc
  fullstats.cc
  handlerbase.cc
  history.cc
  icaanalysisbase.cc
  index.cc
  info.cc
  interpolator1d.cc
  iodata.cc
  ioplugin.cc
  kmeans.cc
  labelmap.cc
  mitestimages.cc
  module.cc
  msgstream.cc 
  minimizer.cc
  nccsum.cc 
  noisegen.cc 
  optionparser.cc 
  optparam.cc
  paramoption.cc  
  parameter.cc  
  paramtranslator.cc  
  pixeltype.cc 
  plugin_base.cc  
  product_base.cc
  productcache.cc
  property_flags.cc
  probmap.cc
  regmodel.cc
  revision.cc 
  scaler1d.cc
  searchpath.cc
  seriesstats.cc 
  selftestcmdoption.cc
  shape.cc 
  slopestatistics.cc
  slopeclassifier.cc
  spacial_kernel.cc
  sparse_histogram.cc
  splinekernel.cc
  splineparzenmi.cc
  sqmin.cc
  streamredir.cc
  testplugin.cc 
  threadedmsg.cc
  typedescr.cc
  utils.cc 
  watch.cc 
  waveletslopeclassifier.cc
  xmlinterface.cc
  )

SET(MIACORE_HEADER_BASE
  ${GSLPP_HEADERS}
  attributes.hh
  attribute_names.hh 
  attributetype.hh
  boundary_conditions.hh
  callback.hh
  cmdbooloption.hh
  cmdlineparser.hh
  cmdoption.hh
  cmdparamoption.hh
  cmdoptionflags.hh
  cmdstringoption.hh
  cmeans.hh
  combiner.hh
  convergence_measure.hh
  cost.hh
  cost.cxx
  combiner.hh
  creator.hh
  cstplan.hh
  datapool.hh
  defines.hh
  delayedparameter.hh
  dictmap.hh
  distance.hh
  dlloader.hh
  dummyhandler.hh
  errormacro.hh
  export_handler.hh
  factory.hh
  factory_trait.hh
  fastica.hh
  fastica_nonlinearity.hh
  fft1d_r2c.hh
  fftslopeclassifier.hh
  fifofilter.hh
  file.hh
  filetools.hh
  filter.hh
  fixedwidthoutput.hh
  flags.hh
  flagstring.hh
  fullstats.hh
  handlerbase.hh
  handler.cxx handler.hh
  history.hh
  histogram.hh
  icaanalysisbase.hh
  index.hh
  import_handler.hh
  iodata.hh
  iohandler.cxx iohandler.hh
  ioplugin.cxx ioplugin.hh
  kmeans.hh
  labelmap.hh
  meanvar.hh
  mitestimages.hh
  module.hh
  msgstream.hh
  minimizer.hh
  noisegen.hh
  nccsum.hh
  optionparser.hh
  optparam.hh
  parallel.hh
  parallelcxx11.hh
  parameter.cxx parameter.hh
  paramoption.hh
  paramtranslator.hh
  pixeltype.hh
  plugin_base.cxx plugin_base.hh
  probmap.hh
  property_flags.hh
  product_base.hh
  productcache.hh
  refholder.hh
  regmodel.hh
  scaler1d.hh
  shape.hh shape.cxx
  searchpath.hh
  seriesstats.hh
  selftestcmdoption.hh
  singular_refobj.hh
  slopestatistics.hh
  slopeclassifier.hh
  slopevector.hh
  splinekernel.hh
  splineparzenmi.hh
  spacial_kernel.hh
  sparse_histogram.hh
  sparse_solver.hh
  sqmin.hh
  statistics.hh
  streamredir.hh
  svector.hh
  testplugin.hh
  threadedmsg.hh
  tools.hh
  traits.hh
  transformation.hh
  typedescr.hh
  type_traits.hh
  utils.hh
  vector.hh
  watch.hh
  waveletslopeclassifier.hh
  xmlinterface.hh
  )

SET(MIACORETEST_SRC 
  test_core_combined.cc
)

IF(ITPP_FOUND)
  SET(ITPP_SRC 
    ica.cc 
    )
  SET(ITPP_HEADER 
    ica.hh
    ica_template.hh ica_template.cxx
    )
ENDIF(ITPP_FOUND)

IF(NOT TBB_FOUND)
  SET(MIACORE_SRC_PARALLELCXX11 
    parallelcxx11.cc
    )
  NEW_TEST(parallelcxx11 miacore)
ENDIF(NOT TBB_FOUND)
  
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
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/revision.hh.cmake ${CMAKE_CURRENT_BINARY_DIR}/revision.hh)


#
# the library target 
#
SET(MIACORE_SRC ${MIACORE_SRC_BASE} ${ITPP_SRC} ${FFTWF_SRC} ${PWPDF_SRC} ${MIACORE_SRC_PARALLELCXX11})
SET(MIACORE_HEADER ${MIACORE_HEADER_BASE} ${ITPP_HEADER} ${FFTWF_HEADER} ${PWPDF_HEADER})

SET(miacore_deps ${BASELIBS} ${TBB_LIBRARIES} ${FFTWF_LIBRARIES} ${XMLPP_LIBRARIES})
MIA_ADD_LIBRARY(miacore "${MIACORE_SRC}" "${miacore_deps}")

IF(PWPDF_FOUND AND FFTWD_FOUND)
  TARGET_LINK_LIBRARIES(miacore ${PWPDF_LIBRARIES})
ENDIF(PWPDF_FOUND AND FFTWD_FOUND)

IF(ITPP_FOUND)
  TARGET_LINK_LIBRARIES(miacore ${ITPP_LIBRARIES})
ENDIF(ITPP_FOUND)

SET(MIACORE miacore ${BASELIBS})
SET_TARGET_PROPERTIES(miacore PROPERTIES 
  SOVERSION ${LIBRARY_SOVERSION_INFO}
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

#IF (NOT WIN32)
#  CORE_TEST(history)
#ENDIF (NOT WIN32)
#CORE_TEST(noisegen)

ADD_SUBDIRECTORY(fastica       )
ADD_SUBDIRECTORY(cmeansinit    )
ADD_SUBDIRECTORY(minimizer     )
ADD_SUBDIRECTORY(noise         )
ADD_SUBDIRECTORY(spacialkernel )
ADD_SUBDIRECTORY(splinekernel  ) 
ADD_SUBDIRECTORY(splinebc  ) 
ADD_SUBDIRECTORY(testplug      )

#tests that fork themselfs 
SET(cmdxmlhelp_params  -r 1 --other o)
NEW_TEST_WITH_PARAM(cmdxmlhelp miacore "${cmdxmlhelp_params}")


NEW_TEST(Vector miacore)
NEW_TEST(attributes miacore)
NEW_TEST(boundary_conditions miacore)
NEW_TEST(callback miacore)
NEW_TEST(convergence_measure miacore)
NEW_TEST(cmdoptionflags miacore)
NEW_TEST(cmdparamoption miacore)
NEW_TEST(cmdlineparser miacore)
NEW_TEST(cmdlineparseroutput miacore)
NEW_TEST(cmeans miacore)
NEW_TEST(datapool miacore)
NEW_TEST(delayedparameter miacore)
NEW_TEST(distance miacore)
NEW_TEST(factoryoption miacore)
NEW_TEST(fastica miacore)
NEW_TEST(fftslopeclassifier miacore)
NEW_TEST(filetools miacore)
NEW_TEST(fixedwidthoutput miacore)
NEW_TEST(flagstring  miacore)
NEW_TEST(fullstats miacore)
NEW_TEST(handler miacore)
NEW_TEST(histogram miacore)
NEW_TEST(index miacore)
NEW_TEST(interpolator1d miacore)
NEW_TEST(iohandler miacore)
NEW_TEST(kmeans miacore)
NEW_TEST(labelmap miacore)
NEW_TEST(meanvar  miacore)
NEW_TEST(nccsum  miacore)
NEW_TEST(productcache  miacore)
NEW_TEST(property_flags  miacore)
NEW_TEST(scaler1d miacore)
NEW_TEST(seriesstats miacore)
NEW_TEST(singular_refobj miacore)
NEW_TEST(shape miacore)
NEW_TEST(simpson miacore)
NEW_TEST(slopeclassifier miacore)
NEW_TEST(slopestatistics miacore)
NEW_TEST(sparse_solver miacore)
NEW_TEST(sparse_histogram miacore)
NEW_TEST(splinekernel miacore)
NEW_TEST(splineparzenmi miacore)
NEW_TEST(streamvector miacore)
NEW_TEST(statistics miacore)
NEW_TEST(threadedmsg miacore)
NEW_TEST(tools miacore)
NEW_TEST(utils miacore)
#NEW_TEST(watch miacore)
NEW_TEST(waveletslopeclassifier miacore)
NEW_TEST(xmlinterface miacore)

NEW_TEST(gsl_matrix miacore)
NEW_TEST(gsl_matrix_vector_ops miacore)
NEW_TEST(gsl_multimin miacore)
NEW_TEST(gsl_pca miacore)
NEW_TEST(gsl_vector miacore)


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
