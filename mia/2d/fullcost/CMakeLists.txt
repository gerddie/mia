SET(fullcost image)
PLUGINGROUP_WITH_TEST_AND_PREFIX(2d-cost "${fullcost}" 
  mia2dtest "${PLUGIN_INSTALL_PATH}/fullcost/2d"
  )
