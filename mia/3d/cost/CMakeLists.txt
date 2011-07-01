SET(cost3d
	   ssd
	   ngf
	   fatssd
	   fatngf
)

PLUGINGROUP_WITH_TEST_AND_PREFIX(3d-cost "${cost3d}" 
  "${MIA3DLIBS}" "${PLUGIN_INSTALL_PATH}/cost/3dimage"
  )
