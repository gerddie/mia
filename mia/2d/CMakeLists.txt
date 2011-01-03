SET(MIA2D_SRC_BASE
  angle.cc
  2DDatafield.cc
  2DVectorfield.cc
  2dfilter.cc
  2DImage.cc
  2dimageio.cc
  2dvfio.cc
  BoundingBox.cc
  cost.cc
  costbase.cc
  correlation_weight.cc 
  creator.cc
  distances.cc
  divcurlmatrix.cc
  imagecostbase.cc
  interpolator.cc 
  fatcost.cc
  fullcost.cc
  filterchain.cc
  model.cc
  modelsolverreg.cc
  multicost.cc
  nfg.cc
  nonrigidregister.cc 
  polygon.cc
  ppmatrix.cc
  rigidregister.cc
  similarity_profile.cc
  shape.cc
  timestep.cc
  transform.cc
  transformio.cc
  transformfactory.cc
  )

IF(FFTWF_FOUND AND WITH_FFTWF)
  SET(MIA2D_FFTW_CODE     
    cstkernel.cc 
    fftkernel.cc )
ENDIF(FFTWF_FOUND AND WITH_FFTWF)

IF(XMLPP_FOUND)
  SET(SEG_SRC 
    SegPoint.cc 
    SegStar.cc 
    SegSection.cc 
    SegFrame.cc 
    SegSet.cc
    SegSetWithImages.cc
    )
  SET(SEG_HEADER 
    SegPoint.hh
    SegStar.hh
    SegSection.hh
    SegFrame.hh
    SegSet.hh
    SegSetWithImages.hh
    )
ENDIF(XMLPP_FOUND)

IF(ITPP_FOUND)
  SET(ITPP_SRC 
    ica.cc 
    perfusion.cc
    )
  SET(ITPP_HEADER 
    ica.hh
    perfusion.hh
    )
ENDIF(ITPP_FOUND)

SET(MIA2D_SRC ${MIA2D_SRC_BASE} ${MIA2D_FFTW_CODE})

SET(MIA2D_HEADERS_BASE
  angle.hh
  2DDatafield.hh 2DDatafield.cxx
  2dfilter.hh
  2DImage.hh
  2dimageio.hh
  2dimageiotest.hh
  2DVector.hh
  2DVectorfield.hh
  2dvfio.hh
  BoundingBox.hh
  cost.hh
  costbase.hh
  correlation_weight.hh 
  creator.hh
  defines2d.hh
  deformer.hh
  distances.hh
  divcurlmatrix.cc
  fatcost.hh
  fullcost.hh
  filterchain.hh
  fftkernel.hh
  ground_truth_evaluator.hh
  groundtruthproblem.hh
  imagecostbase.hh
  interpolator.hh
  matrix.hh
  model.hh
  modelsolverreg.hh
  multicost.hh
  nonrigidregister.hh
  nfg.hh
  perfusion.hh
  polygon.hh
  ppmatrix.hh
  rigidregister.hh
  shape.hh
  similarity_profile.hh
  timestep.hh
  transform.hh
  transformmock.hh
  transformio.hh
  transformfactory.hh
  interpolator.hh interpolator.cxx
  )

SET(MIA2D_HEADERS ${MIA2D_HEADERS_BASE} ${SEG_HEADER} ${ITPP_HEADER})

#
# the main 2D library
# 
ADD_LIBRARY(mia2d SHARED ${MIA2D_SRC})
TARGET_LINK_LIBRARIES(mia2d miacore)

#
# What we need to link from here on 
#
SET(MIA2DLIBS mia2d  ${LIBS})

#
# The test library: On MS windows it needs to be static, 
# because it doesen't resolve all
# symbols, and in Windows, this is not supported
# 
ADD_LIBRARY(mia2dtest SHARED 
  2dimageiotest.cc 
  2dvfiotest.cc 
  filtertest.cc 
  imagetest.cc
  transformmock.cc
  )

TARGET_LINK_LIBRARIES(mia2dtest mia2d ${BOOST_UNITTEST})
SET_TARGET_PROPERTIES(mia2dtest PROPERTIES COMPILE_FLAGS -DBOOST_TEST_DYN_LINK)

SET_TARGET_PROPERTIES(mia2d mia2dtest PROPERTIES 
  VERSION ${LIBRARY_VERSION_INFO}
)

IF(APPLE)
   SET_TARGET_PROPERTIES(mia2dtest PROPERTIES LINK_FLAGS "-single_module -Wl,-flat_namespace -Wl,-undefined -Wl,suppress")
ENDIF(APPLE)

MACRO(TEST_2DMIA file libs)
  ADD_EXECUTABLE(test-2d-${file} test_${file}.cc)
  TARGET_LINK_LIBRARIES(test-2d-${file} ${libs} ${BOOST_UNITTEST} )
  ADD_TEST(2d-${file} test-2d-${file})
ENDMACRO(TEST_2DMIA file)

TEST_2DMIA(angle mia2d)
TEST_2DMIA(2ddatafield mia2dtest)
TEST_2DMIA(2dimage mia2dtest)
TEST_2DMIA(2dparam mia2dtest)
TEST_2DMIA(2dvector mia2dtest)

TEST_2DMIA(boundingbox mia2dtest)

TEST_2DMIA(costbase mia2dtest)
TEST_2DMIA(distances mia2dtest)

TEST_2DMIA(fullcost mia2dtest)
TEST_2DMIA(imagecostbase mia2dtest)
TEST_2DMIA(interpol mia2dtest)

TEST_2DMIA(matrix mia2dtest)
TEST_2DMIA(modelsolverreg mia2dtest)
TEST_2DMIA(nfg mia2dtest)
TEST_2DMIA(nonrigidregister mia2dtest)

TEST_2DMIA(oldnewintegrate mia2dtest)

TEST_2DMIA(polygon mia2dtest)
TEST_2DMIA(ppmatrix mia2dtest)
TEST_2DMIA(rigidregister mia2dtest)
TEST_2DMIA(transformfactory mia2dtest)
TEST_2DMIA(vectorfield_interpolator mia2dtest)
TEST_2DMIA(similarity_profile mia2dtest)
TEST_2DMIA(transio mia2d)
TEST_2DMIA(transform mia2dtest)

IF(CMAKE_BUILD_TYPE)
  SET(CONFIG ${CMAKE_BUILD_TYPE})
ELSE(CMAKE_BUILD_TYPE)
  SET(CONFIG "NOCONFIG")
ENDIF(CMAKE_BUILD_TYPE)

#
# installation 
#
INSTALL_WITH_EXPORT(mia2d)
INSTALL_WITH_EXPORT(mia2dtest)

INSTALL(FILES ${MIA2D_HEADERS} DESTINATION ${INCLUDE_INSTALL_PATH}/mia/2d)



SET(MIA2D_MYOCARDPERF_SRC 
  ${SEG_SRC}
  ${ITPP_SRC}
  ground_truth_evaluator.cc
  groundtruthproblem.cc

)  

SET(MIA2D_MYOCARDPERF_HEADER 
  ${SEG_HEADER}
  ${ITPP_HEADER}
  ground_truth_evaluator.hh
  groundtruthproblem.hh
)  

ADD_LIBRARY(mia2dmyocardperf SHARED ${MIA2D_MYOCARDPERF_SRC})
TARGET_LINK_LIBRARIES(mia2dmyocardperf mia2d)
SET_TARGET_PROPERTIES(mia2dmyocardperf PROPERTIES VERSION ${LIBRARY_VERSION_INFO}
)


IF(XMLPP_FOUND)
  TARGET_LINK_LIBRARIES(mia2dmyocardperf ${XMLPP_LIBRARIES})
ENDIF(XMLPP_FOUND)

INSTALL_WITH_EXPORT(mia2dmyocardperf)
INSTALL(FILES ${MIA2D_MYOCARDPERF_HEADER} DESTINATION ${INCLUDE_INSTALL_PATH}/mia/2d)


IF(ITPP_FOUND)
  TEST_2DMIA(ica mia2dmyocardperf)
  TEST_2DMIA(perfusion mia2dmyocardperf)
ENDIF(ITPP_FOUND)


TEST_2DMIA(groundtruthproblem mia2dmyocardperf)

TEST_2DMIA(correlation_weight mia2dmyocardperf)

IF(XMLPP_FOUND)
  TEST_2DMIA(segmentation mia2dmyocardperf)
  TEST_2DMIA(segframe mia2dmyocardperf)
  TEST_2DMIA(segpoint mia2dmyocardperf)
ENDIF(XMLPP_FOUND)

#
# The plugin directories 
#
ADD_SUBDIRECTORY(cost      )
ADD_SUBDIRECTORY(creator   )
ADD_SUBDIRECTORY(io        )
ADD_SUBDIRECTORY(filter    )
ADD_SUBDIRECTORY(fft       )
ADD_SUBDIRECTORY(fullcost  )
ADD_SUBDIRECTORY(kernel    )
ADD_SUBDIRECTORY(model     )
#ADD_SUBDIRECTORY(reg2d     )
ADD_SUBDIRECTORY(shapes    )
ADD_SUBDIRECTORY(timestep  )
ADD_SUBDIRECTORY(transform )
ADD_SUBDIRECTORY(transio )

