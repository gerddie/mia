SET(iomesh
  off
  ply
  stl
)

PLUGIN_GROUP_PRE(
  "mesh" 
  "${iomesh}" 
  "${MIAMESHLIBS}" 
  "${PLUGIN_INSTALL_PATH}/io/mesh"
)
