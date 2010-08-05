
SET(costplugins2dNew ssd)
PLUGINGROUP_WITH_TEST_AND_PREFIX(2d-cost "${costplugins2dNew}" 
  "${MIA2DLIBS}" "${PLUGIN_INSTALL_PATH}/cost/2dimage"
  )


SET(fatcostplugins2dNew fatngf fatssd fatnssd) 
PLUGINGROUP_WITH_TEST_AND_PREFIX(2d-cost "${fatcostplugins2dNew}" 
  "${MIA2DLIBS}" "${PLUGIN_INSTALL_PATH}/fatcost/2dimage"
  )

SET(generalcostplugins2d ssd2)
PLUGINGROUP_WITH_TEST_AND_PREFIX(2d-cost "${generalcostplugins2d}" 
  "${MIA2DLIBS}" "${PLUGIN_INSTALL_PATH}/cost/2d"
  )

target_link_libraries(test-2d-cost-ssd2 mia2dtest)