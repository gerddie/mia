
SET(costplugins2dNew ssd)
PLUGINGROUP_WITH_TEST_AND_PREFIX(2d-cost "${costplugins2dNew}" 
  "${MIA2DLIBS}" "${PLUGIN_INSTALL_PATH}/cost/2dimage"
  )


SET(fatcostplugins2dNew fatngf fatssd fatnssd) 
PLUGINGROUP_WITH_TEST_AND_PREFIX(2d-cost "${fatcostplugins2dNew}" 
  "${MIA2DLIBS}" "${PLUGIN_INSTALL_PATH}/fatcost/2dimage"
  )

