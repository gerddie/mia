SET(fifof2dNew
  byslice
  gauss
  median
  mlv
  morphological
  regiongrow
)

PLUGINGROUP_WITH_TEST_AND_PREFIX(2dfifo "${fifof2dNew}" 
  "mia3dtest"   "${PLUGIN_INSTALL_PATH}/filter/2dstack"
  )


