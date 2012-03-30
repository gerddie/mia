SET(lmio
  lmx
)
SET(SERLIBS mia3d ${Boost_SERIALIZATION_LIBRARY})

PLUGIN_GROUP_PRE(
  "3d-landmarkio"
  "${lmio}" 
  "${mia3d}" 
  "${PLUGIN_INSTALL_PATH}/3dlandmarklist/io"
)


