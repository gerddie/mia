
SET(regularizers
  fluid-generic
)

PLUGINGROUP_WITH_TEST_AND_PREFIX2("3dvf" "regularizerkernel" "${regularizers}" 
  "${MIA3DLIBS}" TESTLIBS  mia3dtest
  )

