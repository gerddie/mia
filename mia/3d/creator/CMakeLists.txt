SET(creator3d
	    sphere
	    lattic
)

PLUGIN_GROUP_PRE(
  "3d" 
  "${creator3d}" 
  "${MIA3DLIBS}" 
  "${PLUGIN_INSTALL_PATH}/creator/3dimage"
)



