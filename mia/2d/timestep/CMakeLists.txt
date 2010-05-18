SET(timesteps2d
  direct
  fluid
)

PLUGINGROUP_WITH_TEST_AND_PREFIX(2d-timestep "${timesteps2d}" mia2dtest
  "${PLUGIN_INSTALL_PATH}/timestep/2dimage")




