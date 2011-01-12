#todo
SET(io3dtransform_serialized
  bbs 
  xml
)

SET(SERLIBS mia3d ${Boost_SERIALIZATION_LIBRARY})
MESSAGE(STATUS ${SERLIBS})

PLUGIN_GROUP_PRE(
  "3d" 
  "${io3dtransform_serialized}" 
  "${SERLIBS}" 
  "${PLUGIN_INSTALL_PATH}/io/3dtransform"
)
