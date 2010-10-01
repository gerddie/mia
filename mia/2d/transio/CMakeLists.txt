#todo
SET(io2dtransform_serialized
  bbs 
  xml
)


SET(SERLIBS mia2d ${Boost_SERIALIZATION})

PLUGIN_GROUP_PRE(
  "2d" 
  "${io2dtransform_serialized}" 
  "${SERLIBS}" 
  "${PLUGIN_INSTALL_PATH}/io/2dtransform"
)
