#
# This file is part of MIA - a toolbox for medical image analysis 
# Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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


IF(ITPP_FOUND)
  SET(ITPP_SRC 
    ica.cc 
    )
  SET(ITPP_HEADER 
    ica.hh
    )
ENDIF(ITPP_FOUND)


SET(MIA3D_SRC 
  affine_matrix.cc
  camera.cc
  cost.cc
  creator.cc
  critical_point.cc
  2dimagefifofilter.cc
  datafield.cc
  image.cc
  vectorfield.cc
  filter.cc
  imageio.cc
  vfio.cc
  distance.cc
  fullcost.cc
  fuzzyseg.cc
  fuzzyClusterSolverCG.cc
  imagecollect.cc
  interpolator.cc
  landmark.cc
  landmarklist.cc
  landmarklistio.cc
  model.cc
  multicost.cc
  nfg.cc 
  nonrigidregister.cc
  orientation.cc
  quaternion.cc
  ppmatrix.cc 
  register.cc
  rigidregister.cc
  timestep.cc
  similarity_profile.cc
  splinetransformpenalty.cc
  trackpoint.cc
  transform.cc
  transformfactory.cc
  transformio.cc
  shape.cc
  stackdisttrans.cc
  ${ITPP_SRC}
  )

SET(MIA3D_HEADERS
  affine_matrix.hh
  2dimagefifofilter.hh
  datafield.hh datafield.cxx
  vectorfield.hh
  image.hh
  filter.hh
  imageio.hh
  imageiotest.hh
  vector.hh
  vfio.hh
  vfiotest.hh
  camera.hh
  cost.hh
  creator.hh
  critical_point.hh
  defines3d.hh
  deformer.hh
  distance.hh
  fifotestfixture.hh
  fullcost.hh
  fuzzyseg.hh
  fuzzyClusterSolverCG.hh
  interpolator.hh interpolator.cxx
  iterator.hh iterator.cxx
  ica.hh
  imagecollect.hh
  landmark.hh
  landmarklist.hh
  landmarklistio.hh
  matrix.hh
  model.hh
  multicost.hh
  multireg.hh
  nfg.hh
  nonrigidregister.hh
  orientation.hh
  ppmatrix.hh
  quaternion.hh
  register.hh
  rigidregister.hh
  shape.hh 
  similarity_profile.hh
  stackdisttrans.hh
  splinetransformpenalty.hh
  timestep.hh
  trackpoint.hh
  trait.hh
  transform.hh
  transformfactory.hh
  transformio.hh
  transformmock.hh
  ${ITPP_HEADER}
  )

SET(MIA3DTEST_SRC
  test_3d.cc
  test_datafield.cc
  test_image.cc
  test_vector.cc
  test_interpol.cc
  )



#
# The main 3D library
#
MIA_ADD_LIBRARY(mia3d "${MIA3D_SRC}" mia2d)

MACRO(TEST_3D name)
  ADD_EXECUTABLE(test-3d-${name} test_${name}.cc)  
  TARGET_LINK_LIBRARIES(test-3d-${name} mia3dtest ${BOOST_UNITTEST})
  ADD_TEST(3d-${name} test-3d-${name})
  IF(WIN32)
    SET_TARGET_PROPERTIES(test-3d-${name} PROPERTIES LINKER_FLAGS "/NODEFAULTLIB:MSVCRT")
  ENDIF(WIN32)
ENDMACRO(TEST_3D name)


SET(MIA3DLIBS mia3d  ${LIBS})
#
# io test library 
#

SET(mia3dtestsrc 
  imageiotest.cc 
  vfiotest.cc 
  fifotestfixture.cc
  transformmock.cc
  )
SET(mia3dtestdeps ${MIA3DLIBS} mia2dtest ${BOOST_UNITTEST})

MIA_ADD_LIBRARY(mia3dtest "${mia3dtestsrc}" "${mia3dtestdeps}")
SET_TARGET_PROPERTIES(mia3dtest PROPERTIES COMPILE_FLAGS -DBOOST_TEST_DYN_LINK)

IF(APPLE)
  SET_TARGET_PROPERTIES(mia3dtest PROPERTIES LINK_FLAGS "-single_module -Wl,-flat_namespace -Wl,-undefined -Wl,suppress")
ENDIF(APPLE)

#
# Testing 
#
ADD_EXECUTABLE(test-3d ${MIA3DTEST_SRC})
SET_TARGET_PROPERTIES(test-3d PROPERTIES COMPILE_FLAGS -DBOOST_TEST_DYN_LINK)
TARGET_LINK_LIBRARIES(test-3d mia3dtest ${MIA3DLIBS} ${BOOST_UNITTEST})

SET(TEST_3D test-3d)

IF (WIN32)
  ADD_TEST(3d "${CMAKE_CURRENT_SOURCE_DIR}/test-3d.bat")
ELSE (WIN32)
  ADD_TEST(3d test-3d)
  ADD_TEST(3d-imageio test-3dimageio)
ENDIF(WIN32)

TEST_3D(affine_matrix affine_matrix)
TEST_3D(imageio imageio)
TEST_3D(2dimagefifofilter imagefifofilter)
TEST_3D(nfg nfg)
TEST_3D(vectorfield vectorfield)
TEST_3D(orientation orientation)
TEST_3D(deform deform)
IF(ITPP_FOUND)
  TEST_3D(ica ica)
ENDIF(ITPP_FOUND)
TEST_3D(transformfactory transformfactory)
TEST_3D(rigidregister rigidregister)
TEST_3D(nonrigidregister nonrigidregister)
TEST_3D(transform transform)
TEST_3D(transio transio)
TEST_3D(landmark landmark)
TEST_3D(landmarklistio landmarklistio)
TEST_3D(quaternion quaternion)
TEST_3D(ppmatrix ppmatrix)
TEST_3D(matrix matrix)
TEST_3D(fullcost fullcost)
TEST_3D(iterator iterator)
TEST_3D(stackdisttrans stackdisttrans)
TEST_3D(similarity_profile similarity_profile)
TEST_3D(trackpoint similarity_profile)
TEST_3D(distance distance)
TEST_3D(imagecollect imagecollect)
TEST_3D(splinetransformpenalty splinetransformpenalty)
#
# installation 
#
SET(INSTALL_TARGETS mia3d mia3dtest)	
INSTALL_WITH_EXPORT("${INSTALL_TARGETS}")
INSTALL(FILES ${MIA3D_HEADERS} DESTINATION ${INCLUDE_INSTALL_PATH}/mia/3d)

#
# sub directories (plugins) 
#
ADD_SUBDIRECTORY(cost     )
ADD_SUBDIRECTORY(combiner )
ADD_SUBDIRECTORY(creator  )
ADD_SUBDIRECTORY(filter   )
ADD_SUBDIRECTORY(fifof    )
ADD_SUBDIRECTORY(fullcost )
ADD_SUBDIRECTORY(io       )
ADD_SUBDIRECTORY(reg3d    )
ADD_SUBDIRECTORY(shapes   )
ADD_SUBDIRECTORY(splinepenalty  )
ADD_SUBDIRECTORY(transform)
ADD_SUBDIRECTORY(transio)
ADD_SUBDIRECTORY(lmio)
