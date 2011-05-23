SET(MIA3D_SRC 
  cost.cc
  creator.cc
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
  transform.cc
  transformfactory.cc
  transformio.cc
  shape.cc
  stackdisttrans.cc
  )

SET(MIA3D_HEADERS
	cost.hh
	creator.hh
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
	model.hh
	multicost.hh
	nfg.hh
	nonrigidregister.hh
	orientation.hh
	ppmatrix.hh
	quaternion.hh
	register.hh
	rigidregister.hh
	timestep.hh
	transform.hh
	transformfactory.hh
	transformio.hh
	transformmock.hh
	shape.hh 
	stackdisttrans.hh
)

SET(MIA3DTEST_SRC
#	test_cost.cc
#	test_combiner.cc
	test_3d.cc
	test_3ddatafield.cc
#	test_3dfilter.cc
	test_3dimage.cc
#	test_3dimageio.cc
#	test_3dregplugins.cc
	test_3dvector.cc
#	test_3dvfio.cc
	test_interpol.cc
#	test_shape.cc
)

#
# inherit what we always need 
#
SET(LIBS miacore)


#
# The main 3D library
#
ADD_LIBRARY(mia3d SHARED ${MIA3D_SRC})
TARGET_LINK_LIBRARIES(mia3d mia2d ${LIBS})

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

ADD_LIBRARY(mia3dtest SHARED 
  3dimageiotest.cc 
  3dvfiotest.cc 
  fifotestfixture.cc
  transformmock.cc
  )
TARGET_LINK_LIBRARIES(mia3dtest ${MIA3DLIBS} mia2dtest ${BOOST_UNITTEST})
SET_TARGET_PROPERTIES(mia3dtest PROPERTIES COMPILE_FLAGS -DBOOST_TEST_DYN_LINK)

SET_TARGET_PROPERTIES(mia3d mia3dtest PROPERTIES 
  VERSION ${LIBRARY_VERSION_INFO}
)

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
TEST_3D(quaternion quaternion)
TEST_3D(ppmatrix ppmatrix)
TEST_3D(fullcost fullcost)
TEST_3D(iterator iterator)
TEST_3D(stackdisttrans stackdisttrans)

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
