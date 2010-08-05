SET(REG2DLIBS mia2d)

#
# define plug-ins 
#

#SET(models2d 
#
#)

#PLUGIN_GROUP_PRE("2d" "${models2d}" "${MIA2DLIBS}" "${PLUGIN_INSTALL_PATH}/model/2dimage")


SET(modelplugins2d 
  identity
  navier
  naviera
  )

PLUGINGROUP_WITH_TEST_AND_PREFIX(2d-model "${modelplugins2d}" 
  "${MIA2DLIBS}" "${PLUGIN_INSTALL_PATH}/model/2dimage"
  )
