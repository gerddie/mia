#
# Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#
#

MACRO(MIA_ADD_LIBRARY name sources deps)
  ADD_LIBRARY(${name} SHARED ${sources})
  TARGET_LINK_LIBRARIES(${name} ${deps})
  SET_TARGET_PROPERTIES(${name} PROPERTIES 
    SOVERSION ${LIBRARY_SOVERSION_INFO}
    VERSION ${LIBRARY_VERSION_INFO}
    OUTPUT_NAME ${name}-${VERSION}
    )
ENDMACRO(MIA_ADD_LIBRARY)


MACRO(DEFPLUGIN plugin file libs)
    ADD_LIBRARY(${plugin} MODULE ${file})
    TARGET_LINK_LIBRARIES(${plugin} ${libs})
    IF (WIN32)   
      SET_TARGET_PROPERTIES(${plugin} PROPERTIES LINKER_FLAGS "/NODEFAULTLIB:MSVCRT")     
    ENDIF (WIN32)	
    SET_TARGET_PROPERTIES(${plugin} PROPERTIES PREFIX "" SUFFIX ${PLUGSUFFIX})
#    ADD_TEST(${plugin} ${CMAKE_BINARY_DIR}/mia/core/mia-plugin_test -u "./${plugin}${PLUGSUFFIX}")
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
  
  ADD_CUSTOM_TARGET(mia-${name}.xml)
  add_dependencies(mia-${name}.xml testinstall_for_doc)

  if("${CMAKE_GENERATOR}" MATCHES Make)
    ADD_CUSTOM_COMMAND(TARGET mia-${name}.xml
      COMMAND
      sh ${CMAKE_SOURCE_DIR}/doc/make-xml.sh ${name}
          ${CMAKE_BINARY_DIR}/testinstall${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_BINARY_DIR}/testinstall${PLUGIN_SEARCH_PATH} 
          ${CMAKE_BINARY_DIR}/doc 
      )
  else("${CMAKE_GENERATOR}" MATCHES Make)
    ADD_CUSTOM_COMMAND(TARGET mia-${name}.xml
      COMMAND ./mia-${name} --help-xml >${CMAKE_BINARY_DIR}/doc/mia-${name}.xml
      )
  endif("${CMAKE_GENERATOR}" MATCHES Make)
    
  add_dependencies(mia-${name}.xml mia-${name})  
  add_dependencies(xmldocs mia-${name}.xml)  


  ADD_CUSTOM_TARGET(mia-${name}.man)  
  IF(GZIP) 
    ADD_CUSTOM_COMMAND(TARGET  mia-${name}.man 
      COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/doc/miaxml2man.py 
      ${CMAKE_BINARY_DIR}/doc/mia-${name}.xml | ${GZIP} -9  >${CMAKE_BINARY_DIR}/doc/man/mia-${name}.1.gz)
  ELSE(GZIP) 
    ADD_CUSTOM_COMMAND(TARGET  mia-${name}.man 
      COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/doc/miaxml2man.py 
      ${CMAKE_BINARY_DIR}/doc/mia-${name}.xml >${CMAKE_BINARY_DIR}/doc/man/mia-${name}.1.gz)
  ENDIF(GZIP)
  add_dependencies(mia-${name}.man mia-${name}.xml manpath)    
  add_dependencies(manpages mia-${name}.man)    


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
