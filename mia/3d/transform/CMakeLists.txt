


SET(transforms translate) 
PLUGINGROUP_WITH_TEST_AND_PREFIX(3d-transform "${transforms}" 
  "${MIA3DLIBS}" "${PLUGIN_INSTALL_PATH}/transform/3dimage"
  )
