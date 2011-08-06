#todo
SET(io2dtransform_serialized
  bbs 
  xml
)

SET(SERLIBS mia2d ${Boost_SERIALIZATION_LIBRARY})

PLUGIN_GROUP_PRE(
  "2d" 
  "${io2dtransform_serialized}" 
  "${SERLIBS}" 
  "${PLUGIN_INSTALL_PATH}/io/2dtransform"
)

IF(FALSE)
SET(PORT_SERLIBS mia2d portable_binary_archive ${Boost_SERIALIZATION_LIBRARY} ${Boost_SYSTEM})
PLUGIN_GROUP_PRE(
  "2d" 
  "pbs" 
  "${PORT_SERLIBS}" 
  "${PLUGIN_INSTALL_PATH}/io/2dtransform"
)
ENDIF(FALSE)
