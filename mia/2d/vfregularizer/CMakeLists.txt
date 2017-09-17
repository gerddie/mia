
SET(vfsolvers
  sor
)

PLUGINGROUP_WITH_TEST_AND_PREFIX2("2dvf" "regularizer" "${vfsolvers}" 
  "${MIA2DLIBS}" TESTLIBS  mia2dtest
  )
