SET(combiner3d
	labelxmap
)

PLUGIN_GROUP_PRE(
  "3d" 
  "${combiner3d}" 
  "${MIA3DLIBS}" 
  "${PLUGIN_INSTALL_PATH}/combiner/3dimage"
)


