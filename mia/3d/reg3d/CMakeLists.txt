SET(REG3DLIBS mia3d)

SET(models3d 
  navier
  naviera
)

IF (NOT MSVC)
  SET(models3d ${models3d}   navierasse navierpsse)
ENDIF (NOT MSVC)

PLUGIN_GROUP_PRE("3d" "${models3d}" "${REG3DLIBS}" "${PLUGIN_INSTALL_PATH}/model/3dimage")

SET(timesteps3d
  fluid
  direct
)

PLUGIN_GROUP_PRE("3d" "${timesteps3d}" "${REG3DLIBS}" "${PLUGIN_INSTALL_PATH}/timestep/3dimage")

