SET(creator2d
	    circle
)

PLUGIN_GROUP_PRE(
  "2d" 
  "${creator2d}" 
  "${MIA2DLIBS}" 
  "${PLUGIN_INSTALL_PATH}/creator/2dimage"
)
