
SET(regularizers
  fluid-generic
)

PLUGINGROUP_WITH_TEST_AND_PREFIX2("2dvf" "regularizerkernel" "${regularizers}" 
  "${MIA2DLIBS}" TESTLIBS  mia2dtest
  )

