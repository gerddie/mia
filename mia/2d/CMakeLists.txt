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

SET(MIA2D_SRC_BASE
  angle.cc
  boundingbox.cc
  correlation_weight.cc 
  cost.cc
  creator.cc
  cstkernel.cc 
  datafield.cc
  distance.cc
  filter.cc
  image.cc
  imageio.cc
  interpolator.cc 
  fftkernel.cc
  fuzzyseg.cc
  fuzzyclustersolver_sor.cc
  fuzzyclustersolver_cg.cc
  fullcost.cc
  maskedcost.cc 
  model.cc
  morphshape.cc
  multicost.cc
  nfg.cc
  nonrigidregister.cc 
  polygon.cc
  ppmatrix.cc
  rgbimageio.cc
  rigidregister.cc
  shape.cc
  similarity_profile.cc
  sparse_image_solver.cc
  splinetransformpenalty.cc
  timestep.cc
  trackpoint.cc
  transform.cc
  transformio.cc
  transformfactory.cc
  vectorfield.cc
  vfio.cc
  vfregularizer.cc
  vfregularizerkernel.cc
  )


SET(SEG_SRC 
  segframe.cc 
  segpoint.cc 
  segsection.cc 
  segset.cc
  segsetwithimages.cc
  segstar.cc 
  )
SET(SEG_HEADER 
  segpoint.hh
  segstar.hh
  segsection.hh
  segframe.hh
  segset.hh
  segsetwithimages.hh
  )

SET(MIA2D_SRC ${MIA2D_SRC_BASE} ${MIA2D_FFTW_CODE})

SET(MIA2D_HEADERS_BASE
  angle.hh
  boundingbox.hh
  cost.hh
  correlation_weight.hh 
  creator.hh
  cstkernel.hh 
  datafield.hh datafield.cxx
  defines2d.hh
  deformer.hh
  distance.hh
  fftkernel.hh
  filter.hh
  fullcost.hh
  fuzzyseg.hh
  fuzzyclustersolver_sor.hh
  fuzzyclustersolver_cg.hh
  ground_truth_evaluator.hh
  groundtruthproblem.hh
  image.hh
  imageio.hh
  imageiotest.hh
  interpolator.hh interpolator.cxx
  iterator.hh
  iterator.cxx
  matrix.hh
  maskedcost.hh 
  model.hh
  morphshape.hh
  multicost.hh
  nonrigidregister.hh
  nfg.hh
  perfusion.hh
  polygon.hh
  ppmatrix.hh
  rgbimageio.hh
  rigidregister.hh
  shape.hh
  similarity_profile.hh
  sparse_image_solver.hh
  splinetransformpenalty.hh
  timestep.hh
  trait.hh
  trackpoint.hh
  transform.hh
  transformmock.hh
  transformio.hh
  transformfactory.hh
  vector.hh
  vectorfield.hh
  vfio.hh
  vfregularizer.hh
  vfregularizerkernel.hh
  )

SET(MIA2D_HEADERS ${MIA2D_HEADERS_BASE} ${SEG_HEADER} ${ITPP_HEADER})

#
# the main 2D library
# 
MIA_ADD_LIBRARY(mia2d "${MIA2D_SRC}" miacore)

#
# What we need to link from here on 
#
SET(MIA2DLIBS mia2d  ${LIBS})


#
# The test library: On MS windows it needs to be static, 
# because it doesen't resolve all
# symbols, and in Windows, this is not supported# 
SET(mia2dtestsrc 
  imageiotest.cc 
  vfiotest.cc 
  imagetest.cc
  transformmock.cc
  )
SET(mia2dlibdeps mia2d ${BOOST_UNITTEST})

MIA_ADD_LIBRARY(mia2dtest "${mia2dtestsrc}" "${mia2dlibdeps}")
SET_TARGET_PROPERTIES(mia2dtest PROPERTIES COMPILE_FLAGS -DBOOST_TEST_DYN_LINK)


IF(APPLE)
   SET_TARGET_PROPERTIES(mia2dtest PROPERTIES LINK_FLAGS "-single_module -Wl,-flat_namespace -Wl,-undefined -Wl,suppress")
ENDIF(APPLE)

MACRO(TEST_2DMIA file libs)
  ADD_EXECUTABLE(test-2d-${file} test_${file}.cc)
  TARGET_LINK_LIBRARIES(test-2d-${file} ${libs} ${BOOST_UNITTEST} )
  ADD_TEST(2d-${file} test-2d-${file})
ENDMACRO(TEST_2DMIA file)

TEST_2DMIA(angle mia2d)
TEST_2DMIA(datafield mia2dtest)
TEST_2DMIA(image mia2dtest)
TEST_2DMIA(filter mia2dtest)
TEST_2DMIA(param mia2dtest)
TEST_2DMIA(vector mia2dtest)
TEST_2DMIA(boundingbox mia2dtest)
TEST_2DMIA(distance mia2dtest)
TEST_2DMIA(fullcost mia2dtest)
TEST_2DMIA(interpol mia2dtest)
TEST_2DMIA(matrix mia2dtest)
TEST_2DMIA(nfg mia2dtest)
TEST_2DMIA(cost mia2dtest)
TEST_2DMIA(iterator mia2dtest)
TEST_2DMIA(nonrigidregister mia2dtest)
TEST_2DMIA(oldnewintegrate mia2dtest)
TEST_2DMIA(combiner mia2dtest)
TEST_2DMIA(polygon mia2dtest)
TEST_2DMIA(ppmatrix mia2dtest)
TEST_2DMIA(rigidregister mia2dtest)
TEST_2DMIA(transformfactory mia2dtest)
TEST_2DMIA(vectorfield mia2dtest)
TEST_2DMIA(similarity_profile mia2dtest)
TEST_2DMIA(transio mia2d)
TEST_2DMIA(transform mia2dtest)
TEST_2DMIA(shape mia2dtest)
TEST_2DMIA(morphshape mia2dtest)
TEST_2DMIA(imageio mia2dtest)
TEST_2DMIA(sparse_image_solver mia2dtest)
TEST_2DMIA(filter_cast mia2d)
TEST_2DMIA(splinetransformpenalty mia2d)
TEST_2DMIA(trackpoint mia2dtest)

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
  ground_truth_evaluator.cc
  groundtruthproblem.cc
  ica.cc 
  perfusion.cc
  )  

SET(MIA2D_MYOCARDPERF_HEADER 
  ${SEG_HEADER}
  ground_truth_evaluator.hh
  groundtruthproblem.hh
  ica.hh
  perfusion.hh
  )  

MIA_ADD_LIBRARY(mia2dmyocardperf "${MIA2D_MYOCARDPERF_SRC}" mia2d)

INSTALL_WITH_EXPORT(mia2dmyocardperf)
INSTALL(FILES ${MIA2D_MYOCARDPERF_HEADER} DESTINATION ${INCLUDE_INSTALL_PATH}/mia/2d)

IF(ITPP_FOUND)
  TEST_2DMIA(ica mia2dmyocardperf)
  TEST_2DMIA(perfusion mia2dmyocardperf)
ENDIF(ITPP_FOUND)

set(testperflibs mia2dmyocardperf mia2dtest)

IF(MIA_ENABLE_TESTING)
TEST_2DMIA(groundtruthproblem mia2dmyocardperf)
TEST_2DMIA(correlation_weight mia2dmyocardperf)
TEST_2DMIA(segmentation mia2dmyocardperf)
TEST_2DMIA(segframe "${testperflibs}")
TEST_2DMIA(segpoint mia2dmyocardperf)
ENDIF()


#
# The plugin directories 
#
ADD_SUBDIRECTORY(cost      )
ADD_SUBDIRECTORY(combiner  )
ADD_SUBDIRECTORY(creator   )
ADD_SUBDIRECTORY(io        )
ADD_SUBDIRECTORY(filter    )
ADD_SUBDIRECTORY(fft       )
ADD_SUBDIRECTORY(fullcost  )
ADD_SUBDIRECTORY(kernel    )
ADD_SUBDIRECTORY(model     )
ADD_SUBDIRECTORY(maskedcost)
ADD_SUBDIRECTORY(rgbio     )
ADD_SUBDIRECTORY(shapes    )
#ADD_SUBDIRECTORY(sparseimgsolver )
ADD_SUBDIRECTORY(timestep  )
ADD_SUBDIRECTORY(transform )
ADD_SUBDIRECTORY(transio )
ADD_SUBDIRECTORY(splinepenalty )
ADD_SUBDIRECTORY(vfregularizer )
ADD_SUBDIRECTORY(vfregularizerkernel )
