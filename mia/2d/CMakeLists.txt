SET(MIA2D_SRC_BASE
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
  ground_truth_evaluator.cc
  groundtruthproblem.cc
  filterchain.cc
  model.cc
  modelsolverreg.cc
  nfg.cc
  nonrigidregister.cc 
  perfusion.cc
  polygon.cc
  rigidregister.cc
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
    )
  SET(ITPP_HEADER 
    ica.hh
    )
ENDIF(ITPP_FOUND)

SET(MIA2D_SRC ${MIA2D_SRC_BASE} ${MIA2D_FFTW_CODE} ${SEG_SRC} ${ITPP_SRC})

SET(MIA2D_HEADERS_BASE
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
  filterchain.hh
  fftkernel.hh
  ground_truth_evaluator.hh
  groundtruthproblem.hh
  imagecostbase.hh
  interpolator.hh
  matrix.hh
  model.hh
  modelsolverreg.hh
  nonrigidregister.hh
  nfg.hh
  perfusion.hh
  polygon.hh
  rigidregister.hh
  shape.hh
  timestep.hh
  transform.hh
  transformmock.hh
  transformio.hh
  transformfactory.hh
  interpolator.hh interpolator.cxx
  )

SET(MIA2D_HEADERS ${MIA2D_HEADERS_BASE} ${SEG_HEADER} ${ITPP_HEADER})

#
# Libs that we need 
#
SET(LIBS miacore)

#
# the main 2D library
# 
ADD_LIBRARY(mia2d SHARED ${MIA2D_SRC})
TARGET_LINK_LIBRARIES(mia2d ${LIBS} ${GSLPP_LIBRARIES})
IF(XMLPP_FOUND)
  TARGET_LINK_LIBRARIES(mia2d ${XMLPP_LIBRARIES})
ENDIF(XMLPP_FOUND)

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

TARGET_LINK_LIBRARIES(mia2dtest ${MIA2DLIBS} ${BOOST_UNITTEST})
SET_TARGET_PROPERTIES(mia2dtest PROPERTIES COMPILE_FLAGS -DBOOST_TEST_DYN_LINK)

SET_TARGET_PROPERTIES(mia2d mia2dtest PROPERTIES 
  VERSION ${LIBRARY_VERSION_INFO}
)

IF(APPLE)
   SET_TARGET_PROPERTIES(mia2dtest PROPERTIES LINK_FLAGS "-single_module -Wl,-flat_namespace -Wl,-undefined -Wl,suppress")
ENDIF(APPLE)

MACRO(TEST_2DMIA file)
  ADD_EXECUTABLE(test-2d-${file} test_${file}.cc)
  TARGET_LINK_LIBRARIES(test-2d-${file} mia2dtest ${BOOST_UNITTEST})
  ADD_TEST(2d-${file} test-2d-${file})
ENDMACRO(TEST_2DMIA file)

TEST_2DMIA(2dparam)
TEST_2DMIA(2dvector)
TEST_2DMIA(vectorfield_interpolator)
TEST_2DMIA(2ddatafield)
TEST_2DMIA(2dimage)
TEST_2DMIA(interpol)
TEST_2DMIA(matrix)
TEST_2DMIA(transformfactory)
TEST_2DMIA(nonrigidregister)
TEST_2DMIA(nfg)
TEST_2DMIA(modelsolverreg)
TEST_2DMIA(polygon)
TEST_2DMIA(boundingbox)
TEST_2DMIA(distances)
TEST_2DMIA(oldnewintegrate)
TEST_2DMIA(rigidregister)
TEST_2DMIA(perfusion)

TEST_2DMIA(costbase)
TEST_2DMIA(imagecostbase)
TEST_2DMIA(correlation_weight)
TEST_2DMIA(groundtruthproblem)

TEST_2DMIA(divcurlmatrix)

IF(ITPP_FOUND)
  TEST_2DMIA(ica)
ENDIF(ITPP_FOUND)


IF(XMLPP_FOUND)
  TEST_2DMIA(segmentation)
ENDIF(XMLPP_FOUND)

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

#
# The plugin directories 
#
ADD_SUBDIRECTORY(cost      )
ADD_SUBDIRECTORY(creator   )
ADD_SUBDIRECTORY(io        )
ADD_SUBDIRECTORY(filter    )
ADD_SUBDIRECTORY(fft       )
ADD_SUBDIRECTORY(kernel    )
ADD_SUBDIRECTORY(model     )
#ADD_SUBDIRECTORY(reg2d     )
ADD_SUBDIRECTORY(shapes    )
ADD_SUBDIRECTORY(timestep  )
ADD_SUBDIRECTORY(transform )
