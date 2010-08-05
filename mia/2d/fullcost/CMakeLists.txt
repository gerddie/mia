SET(fullcost image divcurl)
PLUGINGROUP_WITH_TEST_AND_PREFIX(2d-fullcost "${fullcost}" 
  mia2dtest "${PLUGIN_INSTALL_PATH}/fullcost/2d"
  )
