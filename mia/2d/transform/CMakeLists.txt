


SET(transforms affine spline vectorfield) 
PLUGINGROUP_WITH_TEST_AND_PREFIX(2d-transform "${transforms}" 
  "${MIA2DLIBS}" "${PLUGIN_INSTALL_PATH}/transform/2dimage"
  )
