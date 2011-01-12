SET(transforms affine rigid spline translate vectorfield) 
PLUGINGROUP_WITH_TEST_AND_PREFIX(2d-transform "${transforms}" 
  "${MIA2DLIBS}" "${PLUGIN_INSTALL_PATH}/transform/2dimage"
  )

