SET(shape3d 
	    3d-shapes
)


ADD_LIBRARY(3d-shapes MODULE basic_shapes.cc sphere.cc)
TARGET_LINK_LIBRARIES(3d-shapes ${MIA3DLIBS})
SET_TARGET_PROPERTIES(3d-shapes PROPERTIES PREFIX "")
INSTALL(TARGETS 3d-shapes LIBRARY DESTINATION "${PLUGIN_INSTALL_PATH}/shape/3dimage")
ADD_TEST(3d-shapes ../../core/mia-plugin_test -u ./3d-shapes${PLUGSUFFIX})
