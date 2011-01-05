SET(cost3d
	   ssd
)

PLUGINGROUP_WITH_TEST_AND_PREFIX(3d-cost "${cost3d}" 
  "${MIA3DLIBS}" "${PLUGIN_INSTALL_PATH}/cost/3dimage"
  )

SET(fatcostplugins3d
	    fatngf
	    fatssd
)

PLUGIN_GROUP_PRE("3d" "${fatcostplugins3d}" "${MIA3DLIBS}" "${PLUGIN_INSTALL_PATH}/fatcost/3dimage")


