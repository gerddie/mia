SET(cost3d
	   ssd
	   ngf
)

PLUGINGROUP_WITH_TEST_AND_PREFIX(3d-cost "${cost3d}" 
  "${MIA3DLIBS}" "${PLUGIN_INSTALL_PATH}/cost/3dimage"
  )


SET(fatcost3d fatngf fatssd) 

PLUGINGROUP_WITH_TEST_AND_PREFIX(3d-cost "${fatcost3d}" 
  "${MIA3DLIBS}" "${PLUGIN_INSTALL_PATH}/fatcost/3dimage"
  )

