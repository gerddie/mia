MACRO(DEFPLUGIN plugin file libs)
    ADD_LIBRARY(${plugin} MODULE ${file})
    TARGET_LINK_LIBRARIES(${plugin} ${libs})
    IF (WIN32)   
      SET_TARGET_PROPERTIES(${plugin} PROPERTIES LINKER_FLAGS "/NODEFAULTLIB:MSVCRT")     
    ENDIF (WIN32)	
    SET_TARGET_PROPERTIES(${plugin} PROPERTIES PREFIX "" SUFFIX ${PLUGSUFFIX})
    ADD_TEST(${plugin} ${CMAKE_BINARY_DIR}/mia/core/mia-plugin_test -u "./${plugin}${PLUGSUFFIX}")
ENDMACRO(DEFPLUGIN)

MACRO(DEFCPLUGIN plugin file libs)
    ADD_LIBRARY(${plugin} MODULE ${file})
    TARGET_LINK_LIBRARIES(${plugin} ${libs})
    IF (WIN32)   
      SET_TARGET_PROPERTIES(${plugin} PROPERTIES LINKER_FLAGS "/NODEFAULTLIB:MSVCRT")     
    ENDIF (WIN32)	
    SET_TARGET_PROPERTIES(${plugin} PROPERTIES PREFIX "" SUFFIX ${PLUGSUFFIX})
ENDMACRO(DEFCPLUGIN)

MACRO(DEFPLUGIN_NO_TEST plugin file libs)
    ADD_LIBRARY(${plugin} MODULE ${file})
    TARGET_LINK_LIBRARIES(${plugin} ${libs})
    IF (WIN32)   
      SET_TARGET_PROPERTIES(${plugin} PROPERTIES LINKER_FLAGS "/NODEFAULTLIB:MSVCRT")     
    ENDIF (WIN32)	
    SET_TARGET_PROPERTIES(${plugin} PROPERTIES PREFIX "" SUFFIX ${PLUGSUFFIX})
ENDMACRO(DEFPLUGIN_NO_TEST)

MACRO(PLUGIN plugin libs install_path)
  DEFPLUGIN(${plugin} ${plugin}.cc "${libs}")
  INSTALL(TARGETS ${plugin} LIBRARY DESTINATION ${install_path})
ENDMACRO(PLUGIN)

MACRO(PLUGIN_PRE prefix plugin libs install_path)
	DEFPLUGIN(${prefix}-${plugin} ${plugin}.cc "${libs}")
        INSTALL(TARGETS ${prefix}-${plugin}
		LIBRARY DESTINATION ${install_path})
ENDMACRO(PLUGIN_PRE)


MACRO(CPLUGIN plugin libs install_path)
	DEFCPLUGIN(${plugin} ${plugin}.c "${libs}")
        INSTALL(TARGETS ${plugin}
		LIBRARY DESTINATION ${install_path})
ENDMACRO(CPLUGIN)


MACRO(PLUGIN_GROUP_NOINST plugins libs)
    FOREACH(p ${plugins})
	DEFPLUGIN_NO_TEST(${p} ${p}.cc "${libs}")
    ENDFOREACH(p)
ENDMACRO(PLUGIN_GROUP_NOINST)


MACRO(PLUGIN_GROUP plugins libs install_path)
    PLUGIN_GROUP_NOINST("${plugins}" "${libs}")
    INSTALL(TARGETS ${plugins} LIBRARY DESTINATION ${install_path})
ENDMACRO(PLUGIN_GROUP)


MACRO(PLUGIN_GROUP_PRE_NOINST prefix plugins libs)
  FOREACH(p ${plugins})
    DEFPLUGIN(${prefix}-${p} ${p}.cc "${libs}")
  ENDFOREACH(p)
ENDMACRO(PLUGIN_GROUP_PRE_NOINST)

MACRO(PLUGIN_GROUP_PRE prefix plugins libs install_path)
  PLUGIN_GROUP_PRE_NOINST(${prefix}  "${plugins}" "${libs}")
  FOREACH(p ${plugins})
    INSTALL(TARGETS ${prefix}-${p} LIBRARY DESTINATION ${install_path})
  ENDFOREACH(p)
  IF(WARN_OLD_PLUGINSTYLE)
    MESSAGE("Plugins '${plugins}' with target '${install_path}' use old interface")
  ENDIF(WARN_OLD_PLUGINSTYLE)
ENDMACRO(PLUGIN_GROUP_PRE)


MACRO(INSTALL_WITH_EXPORT lib)
    INSTALL(TARGETS ${lib} 
      EXPORT Mia
      RUNTIME DESTINATION "bin"
      LIBRARY DESTINATION "lib"
      ARCHIVE DESTINATION "lib")  
ENDMACRO(INSTALL_WITH_EXPORT libs)

MACRO(INSTALL_BASE libs)
  INSTALL(TARGETS ${libs} 
    RUNTIME DESTINATION "bin"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib")
ENDMACRO (INSTALL_BASE)

MACRO(ASSERT_SIZE  NAME EXPECTED)
  CHECK_TYPE_SIZE(${NAME} ${NAME}_TYPE_SIZE)
  IF (NOT ${${NAME}_TYPE_SIZE} EQUAL ${EXPECTED})
    MESSAGE(ERROR, "'${NAME}' size ${${NAME}_TYPE_SIZE}, expected ${EXPECTED}")
  ENDIF(NOT ${${NAME}_TYPE_SIZE} EQUAL ${EXPECTED})
ENDMACRO(ASSERT_SIZE)


MACRO(DEFEXE name deps ) 
   ADD_EXECUTABLE(mia-${name} ${name}.cc)
   
   FOREACH(lib ${deps}) 
       TARGET_LINK_LIBRARIES(mia-${name} ${lib})
   ENDFOREACH(lib)

   TARGET_LINK_LIBRARIES(mia-${name} ${BASELIBS})
   INSTALL(TARGETS mia-${name} RUNTIME DESTINATION "bin")
ENDMACRO(DEFEXE)

MACRO(DEFCHKEXE name deps ) 
   ADD_EXECUTABLE(mia-${name} ${name}.cc)
   
   FOREACH(lib ${deps}) 
       TARGET_LINK_LIBRARIES(mia-${name} ${lib})
   ENDFOREACH(lib)
   SET_TARGET_PROPERTIES(mia-${name} PROPERTIES COMPILE_FLAGS -DVSTREAM='\\\"TEST-2D\\\"' COMPILE_FLAGS -DBOOST_TEST_DYN_LINK)
   TARGET_LINK_LIBRARIES(mia-${name} ${BASELIBS})
   TARGET_LINK_LIBRARIES(mia-${name} ${BOOST_UNITTEST})
   INSTALL(TARGETS mia-${name} RUNTIME DESTINATION "bin")
ENDMACRO(DEFCHKEXE)


MACRO(NEW_TEST name libs)
  SET(EXENAME test-${name})
  
  ADD_EXECUTABLE(${EXENAME} test_${name}.cc)
  TARGET_LINK_LIBRARIES(${EXENAME} ${libs})
  IF (NOT WIN32) 
    TARGET_LINK_LIBRARIES(${EXENAME} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
  ENDIF (NOT WIN32)
  ADD_TEST(${name} ${EXENAME})
ENDMACRO(NEW_TEST)
