
SET(combiners
  ops)

PLUGINGROUP_WITH_TEST_AND_PREFIX(2d-combiner "${combiners}" 
  "${MIA2DLIBS}" "${PLUGIN_INSTALL_PATH}/combiner/2dimage"
  )
