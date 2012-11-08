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

MACRO(PARSE_ARGUMENTS prefix arg_names option_names)
  SET(DEFAULT_ARGS)
  FOREACH(arg_name ${arg_names})    
    SET(${prefix}_${arg_name})
  ENDFOREACH(arg_name)
  FOREACH(option ${option_names})
    SET(${prefix}_${option} FALSE)
  ENDFOREACH(option)

  SET(current_arg_name DEFAULT_ARGS)
  SET(current_arg_list)
  FOREACH(arg ${ARGN})            
    SET(larg_names ${arg_names})    
    LIST(FIND larg_names "${arg}" is_arg_name)                   
    IF (is_arg_name GREATER -1)
      SET(${prefix}_${current_arg_name} ${current_arg_list})
      SET(current_arg_name ${arg})
      SET(current_arg_list)
    ELSE (is_arg_name GREATER -1)
      SET(loption_names ${option_names})    
      LIST(FIND loption_names "${arg}" is_option)            
      IF (is_option GREATER -1)
	     SET(${prefix}_${arg} TRUE)
      ELSE (is_option GREATER -1)
	     SET(current_arg_list ${current_arg_list} ${arg})
      ENDIF (is_option GREATER -1)
    ENDIF (is_arg_name GREATER -1)
  ENDFOREACH(arg)
  SET(${prefix}_${current_arg_name} ${current_arg_list})
ENDMACRO(PARSE_ARGUMENTS)


MACRO(CREATE_PLUGIN_COMMON plugname files libs) 
  add_library(${plugname}-common STATIC "${files}")
  IF(NOT WIN32)
	set_source_files_properties(${files}  PROPERTIES COMPILE_FLAGS "-fPIC")
	set_target_properties(${plugname}-common  PROPERTIES COMPILE_FLAGS -DVSTREAM_DOMAIN='"${plugname}"') 
  ENDIF(NOT WIN32)
  target_link_libraries(${plugname}-common ${libs})
ENDMACRO(CREATE_PLUGIN_COMMON plugname libs) 

MACRO(CREATE_PLUGIN_MODULE plugname)
#  add_library(${plugname} MODULE NO_SOURCE_FILES)
  add_library(${plugname} MODULE ${CMAKE_SOURCE_DIR}/mia/core/silence_cmake_missing_source_file_warning.c)
#  MESSAGE("Remark: Ignore this warning, calling ADD_LIBRARY without source files was done intentionally.")
  set_target_properties(${plugname} PROPERTIES 
    PREFIX ""  
    SUFFIX ${PLUGSUFFIX})
  IF(NOT WIN32)
    set_target_properties(${plugname} PROPERTIES 
      LINK_FLAGS "-Wl,--no-gc-sections -Wl,--undefined,get_plugin_interface"
      )
  ENDIF(NOT WIN32)
  target_link_libraries(${plugname} ${plugname}-common)
ENDMACRO(CREATE_PLUGIN_MODULE plugname)

MACRO(CREATE_PLUGIN_TEST plugname file)
  PARSE_ARGUMENTS(PLUGIN "TESTLIBS" "" ${ARGN})
  add_executable(test-${plugname} ${file})
  IF(NOT WIN32)
    set_target_properties(test-${plugname} PROPERTIES 
      COMPILE_FLAGS -DVSTREAM_DOMAIN='"${plugname}"' 
      COMPILE_FLAGS -DBOOST_TEST_DYN_LINK)
  ELSE(NOT WIN32)
    set_target_properties(test-${plugname} PROPERTIES
      COMPILE_FLAGS -DBOOST_TEST_DYN_LINK)
  ENDIF(NOT WIN32)
  target_link_libraries(test-${plugname} ${plugname}-common)
  target_link_libraries(test-${plugname} ${BOOST_UNITTEST} "${PLUGIN_TESTLIBS}")
  add_test(${plugname} test-${plugname})
ENDMACRO(CREATE_PLUGIN_TEST plugname file)

MACRO(PLUGIN_WITH_TEST plugname file libs)
  PARSE_ARGUMENTS(PLUGIN "TESTLIBS" "" ${ARGN})
  CREATE_PLUGIN_COMMON(${plugname} ${file} "${libs}")
  CREATE_PLUGIN_MODULE(${plugname})
  CREATE_PLUGIN_TEST(${plugname} test_${file} TESTLIBS "${PLUGIN_TESTLIBS}")
ENDMACRO(PLUGIN_WITH_TEST  plugname file libs)

MACRO(PLUGIN_WITH_TEST_AND_PREFIX_NOINST prefix plugname libs)
  PARSE_ARGUMENTS(PLUGIN "TESTLIBS" "" ${ARGN})
  SET(name ${prefix}-${plugname})
  CREATE_PLUGIN_COMMON(${name} ${plugname}.cc "${libs}")
  CREATE_PLUGIN_MODULE(${name})
  CREATE_PLUGIN_TEST(${name} test_${plugname}.cc TESTLIBS "${PLUGIN_TESTLIBS}")
ENDMACRO(PLUGIN_WITH_TEST_AND_PREFIX_NOINST  prefix  plugname file libs)


MACRO(PLUGIN_WITH_TEST_AND_PREFIX prefix plugname libs install_path)
  PARSE_ARGUMENTS(PLUGIN "TESTLIBS" "" ${ARGN})
  SET(name ${prefix}-${plugname})
  CREATE_PLUGIN_COMMON(${name} ${plugname}.cc "${libs}")
  CREATE_PLUGIN_MODULE(${name})
  CREATE_PLUGIN_TEST(${name} test_${plugname}.cc TESTLIBS "${PLUGIN_TESTLIBS}")
  ADD_CUSTOM_TARGET(${name}_test_link ln -sf "${CMAKE_CURRENT_BINARY_DIR}/${name}.mia" 
    ${PLUGIN_TEST_ROOT}/${install_path}/ DEPENDS ${prefix}_testdir ${name})
  ADD_DEPENDENCIES(plugin_test_links ${name}_test_link)
  INSTALL(TARGETS ${name} LIBRARY DESTINATION ${install_path})
ENDMACRO(PLUGIN_WITH_TEST_AND_PREFIX  prefix plugname file libs)


MACRO(SINGLEPLUGIN_WITH_TEST_AND_PREFIX prefix plugname libs install_path)
  PARSE_ARGUMENTS(PLUGIN "TESTLIBS" "" ${ARGN})
  SET(name ${prefix}-${plugname})
  ADD_CUSTOM_TARGET(${prefix}_testdir mkdir -p ${PLUGIN_TEST_ROOT}/${install_path})
  CREATE_PLUGIN_COMMON(${name} ${plugname}.cc "${libs}")
  CREATE_PLUGIN_MODULE(${name})
  CREATE_PLUGIN_TEST(${name} test_${plugname}.cc TESTLIBS "${PLUGIN_TESTLIBS}")
  ADD_CUSTOM_TARGET(${name}_test_link ln -sf "${CMAKE_CURRENT_BINARY_DIR}/${name}.mia" 
    ${PLUGIN_TEST_ROOT}/${install_path}/ DEPENDS ${prefix}_testdir ${name})
  ADD_DEPENDENCIES(plugin_test_links ${name}_test_link)
  INSTALL(TARGETS ${name} LIBRARY DESTINATION ${install_path})
ENDMACRO(SINGLEPLUGIN_WITH_TEST_AND_PREFIX  prefix plugname file libs)

MACRO(PLUGINGROUP_WITH_TEST_AND_PREFIX prefix plugins libs install_path)
  PARSE_ARGUMENTS(PLUGIN "TESTLIBS" "" ${ARGN})
  ADD_CUSTOM_TARGET(${prefix}_testdir mkdir -p ${PLUGIN_TEST_ROOT}/${install_path})
  FOREACH(p ${plugins})
    set(plugname ${prefix}-${p})
    PLUGIN_WITH_TEST(${plugname} ${p}.cc "${libs}" TESTLIBS "${PLUGIN_TESTLIBS}")
    INSTALL(TARGETS ${plugname} LIBRARY DESTINATION ${install_path})
    ADD_CUSTOM_TARGET(${plugname}_test_link ln -sf "${CMAKE_CURRENT_BINARY_DIR}/${plugname}.mia" 
      ${PLUGIN_TEST_ROOT}/${install_path}/ DEPENDS ${prefix}_testdir ${plugname})
    ADD_DEPENDENCIES(plugin_test_links ${plugname}_test_link)
  ENDFOREACH(p)
ENDMACRO(PLUGINGROUP_WITH_TEST_AND_PREFIX)



