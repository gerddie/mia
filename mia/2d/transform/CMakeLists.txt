SET(transforms affine rigid spline translate vf) 
PLUGINGROUP_WITH_TEST_AND_PREFIX(3d-transform "${transforms}" 
  "${MIA2DLIBS}" "${PLUGIN_INSTALL_PATH}/transform/2dimage"
  )

