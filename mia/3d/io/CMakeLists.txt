SET(io3dimage
  analyze 
  inria
  vff
  vtk
)

PLUGIN_GROUP_PRE(
  "3d" 
  "${io3dimage}" 
  "${MIA3DLIBS}" 
  "${PLUGIN_INSTALL_PATH}/io/3dimage"
)


SET(io3vf
  vtkvf
)

PLUGIN_GROUP_PRE(
  "3d" 
  "${io3vf}" 
  "${MIA3DLIBS}" 
  "${PLUGIN_INSTALL_PATH}/io/3dvf"
)
