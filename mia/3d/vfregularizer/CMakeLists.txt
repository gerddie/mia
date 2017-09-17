
SET(vfsolvers
  sor
)

PLUGINGROUP_WITH_TEST_AND_PREFIX2("3dvf" "regularizer" "${vfsolvers}" 
  "${MIA3DLIBS}" TESTLIBS  mia3dtest
  )
