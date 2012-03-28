#
# Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
#
# This program is free software; you can redistribute it and/or modify
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
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#
#

SET(MIA3D_SRC 
  cost.cc
  creator.cc
  critical_point.cc
  2dimagefifofilter.cc
  3DDatafield.cc
  3DImage.cc
  3DVectorfield.cc
  3dfilter.cc
  3dimageio.cc
  3dvfio.cc
  fatcost.cc
  fullcost.cc
  fuzzyseg.cc
  fuzzyClusterSolverCG.cc
  ica.cc
  interpolator.cc
  landmark.cc
  landmarklist.cc
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
  transform.cc
  transformfactory.cc
  transformio.cc
  shape.cc
  stackdisttrans.cc
  )

SET(MIA3D_HEADERS
	cost.hh
	creator.hh
	critical_point.hh
	defines3d.hh
	3DDatafield.hh 3DDatafield.cxx
	3DVectorfield.hh
	3DImage.hh
	3dfilter.hh
	3dimageio.hh
	3dimageiotest.hh
	3DVector.hh
	3dvfio.hh
	3dvfiotest.hh
	fatcost.hh
	fullcost.hh
	fuzzyseg.hh
	fuzzyClusterSolverCG.hh
	interpolator.hh interpolator.cxx
	iterator.hh iterator.cxx
	ica.hh
	landmark.hh
	landmarklist.hh
	matrix.hh
	model.hh
	multicost.hh
	nfg.hh
	nonrigidregister.hh
	orientation.hh
	ppmatrix.hh
	quaternion.hh
	register.hh
	rigidregister.hh
	similarity_profile.hh
	timestep.hh
	trait.hh
	transform.hh
	transformfactory.hh
	transformio.hh
	transformmock.hh
	shape.hh 
	stackdisttrans.hh
)

SET(MIA3DTEST_SRC
	test_3d.cc
	test_3ddatafield.cc
	test_3dimage.cc
	test_3dvector.cc
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
  3dimageiotest.cc 
  3dvfiotest.cc 
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

ADD_EXECUTABLE(test-3dimageio test_3dimageio.cc)
TARGET_LINK_LIBRARIES(test-3dimageio mia3dtest ${MIA3DLIBS} ${BOOST_UNITTEST})

SET(TEST_3D test-3d)

IF (WIN32)
  ADD_TEST(3d "${CMAKE_CURRENT_SOURCE_DIR}/test-3d.bat")
ELSE (WIN32)
  ADD_TEST(3d test-3d)
  ADD_TEST(3d-imageio test-3dimageio)
ENDIF(WIN32)

TEST_3D(2dimagefifofilter 2dimagefifofilter)
TEST_3D(nfg nfg)
TEST_3D(vectorfield vectorfield)
TEST_3D(orientation orientation)
TEST_3D(deform deform)
TEST_3D(ica ica)
TEST_3D(transformfactory transformfactory)
TEST_3D(rigidregister rigidregister)
TEST_3D(nonrigidregister nonrigidregister)
TEST_3D(transform transform)
TEST_3D(transio transio)
TEST_3D(landmark landmark)
TEST_3D(quaternion quaternion)
TEST_3D(ppmatrix ppmatrix)
TEST_3D(matrix matrix)
TEST_3D(fullcost fullcost)
TEST_3D(iterator iterator)
TEST_3D(stackdisttrans stackdisttrans)
TEST_3D(similarity_profile similarity_profile)
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
ADD_SUBDIRECTORY(shapes   )
ADD_SUBDIRECTORY(reg3d    )
ADD_SUBDIRECTORY(transform)
ADD_SUBDIRECTORY(transio)
