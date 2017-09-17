#
# Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

MACRO(CREATE_PLUGIN_COMMON plugname files) 
  add_library(${plugname}-common OBJECT "${files}")
  IF(NOT WIN32)
	set_source_files_properties(${files}  PROPERTIES COMPILE_FLAGS "-fPIC")
	set_target_properties(${plugname}-common  PROPERTIES COMPILE_FLAGS -DVSTREAM_DOMAIN='"${plugname}"') 
  ENDIF(NOT WIN32)
#  target_link_libraries(${plugname}-common ${libs})
ENDMACRO(CREATE_PLUGIN_COMMON plugname) 

MACRO(CREATE_PLUGIN_MODULE_OLD plugname libs)
  add_library(${plugname} MODULE $<TARGET_OBJECTS:${plugname}-common>)
  set_target_properties(${plugname} PROPERTIES 
    PREFIX ""  
    SUFFIX ${PLUGSUFFIX})
  target_link_libraries(${plugname} ${libs})
ENDMACRO(CREATE_PLUGIN_MODULE_OLD plugname)

MACRO(CREATE_PLUGIN_MODULE plugname libs plugindir)
  add_library(${plugname} MODULE $<TARGET_OBJECTS:${plugname}-common>)
  set_target_properties(${plugname} PROPERTIES 
    PREFIX ""  
    SUFFIX ${PLUGSUFFIX}
    LIBRARY_OUTPUT_DIRECTORY "${plugindir}"
    )
  target_link_libraries(${plugname} ${libs})
  ADD_DEPENDENCIES(plugin_test_links ${plugname})
ENDMACRO(CREATE_PLUGIN_MODULE plugname)


MACRO(CREATE_PLUGIN_TEST plugname file libs)
  IF(MIA_ENABLE_TESTING)
    PARSE_ARGUMENTS(PLUGIN "TESTLIBS" "" ${ARGN})
    add_executable(test-${plugname} ${file} $<TARGET_OBJECTS:${plugname}-common>)
    IF(NOT WIN32)
      set_target_properties(test-${plugname} PROPERTIES
        COMPILE_FLAGS -DVSTREAM_DOMAIN='"${plugname}"'
        COMPILE_FLAGS -DBOOST_TEST_DYN_LINK)
    ELSE(NOT WIN32)
      set_target_properties(test-${plugname} PROPERTIES
        COMPILE_FLAGS -DBOOST_TEST_DYN_LINK)
    ENDIF(NOT WIN32)
    target_link_libraries(test-${plugname} ${libs} ${BOOST_UNITTEST} "${PLUGIN_TESTLIBS}")
    add_test(${plugname} test-${plugname})
  ENDIF()
ENDMACRO(CREATE_PLUGIN_TEST plugname file)

MACRO(PLUGIN_WITH_TEST plugname file libs plugindir)
  PARSE_ARGUMENTS(PLUGIN "TESTLIBS" "" ${ARGN})
  CREATE_PLUGIN_COMMON(${plugname} ${file} )
  CREATE_PLUGIN_MODULE(${plugname} "${libs}" ${plugindir})
  CREATE_PLUGIN_TEST(${plugname} test_${file} "${libs}" TESTLIBS "${PLUGIN_TESTLIBS}")
ENDMACRO(PLUGIN_WITH_TEST plugname file libs)

MACRO(PLUGIN_WITH_TEST_AND_PREFIX_NOINST prefix plugname libs)
  PARSE_ARGUMENTS(PLUGIN "TESTLIBS" "" ${ARGN})
  SET(name ${prefix}-${plugname})
  CREATE_PLUGIN_COMMON(${name} ${plugname}.cc "${libs}")
  CREATE_PLUGIN_MODULE_OLD(${name})
  CREATE_PLUGIN_TEST(${name} test_${plugname}.cc TESTLIBS "${PLUGIN_TESTLIBS}")
ENDMACRO(PLUGIN_WITH_TEST_AND_PREFIX_NOINST  prefix  plugname file libs)

MACRO(DEFINE_PLUGIN_NAMES type data rootdir) 
  FOREACH(d ${data})
    SET(${type}_${d}_prefix "${type}-${d}")
    SET(${type}_${d}_dir "${rootdir}/${type}/${d}")
    SET(${type}_${d}_binary "${type}/${d}")
  ENDFOREACH(d)
ENDMACRO(DEFINE_PLUGIN_NAMES type data rootdir) 


MACRO(TEST_PREFIX type data)
  string(COMPARE EQUAL  "x${${type}_${data}_prefix}" "x" UNDEFINED_PREFIX)
  IF(UNDEFINED_PREFIX)
    MESSAGE(FATAL_ERROR "PLUGIN_WITH_TEST_AND_PREFIX2: the prefix for ${type}-${data} is not defined")
ENDIF(UNDEFINED_PREFIX)
ENDMACRO(TEST_PREFIX type data)

MACRO(PLUGIN_WITH_PREFIX2 type data plugname libs)
  TEST_PREFIX(${type} ${data})
  SET(install_path ${${type}_${data}_dir})
  SET(plugin_build_path ${${type}_${data}_binary})
  PARSE_ARGUMENTS(PLUGIN "TESTLIBS" "" ${ARGN})
  SET(name ${${type}_${data}_prefix}-${plugname})
  CREATE_PLUGIN_COMMON(${name} ${plugname}.cc)
  CREATE_PLUGIN_MODULE(${name} "${libs}" ${PLUGIN_TEST_ROOT}/${plugin_build_path})
  INSTALL(TARGETS ${name} LIBRARY DESTINATION ${install_path})
  IF(WARN_MISSING_OR_OLD_PLUGINTESTS)
  MESSAGE("WARNING: Plugin ${name} does provide no or only old-style testing")
  ENDIF(WARN_MISSING_OR_OLD_PLUGINTESTS)
ENDMACRO(PLUGIN_WITH_PREFIX2 type data plugname libs)


MACRO(PLUGIN_WITH_TEST_AND_PREFIX2 type data plugname libs)
  TEST_PREFIX(${type} ${data})
  SET(install_path ${${type}_${data}_dir})
  SET(plugin_build_path ${${type}_${data}_binary})
  PARSE_ARGUMENTS(PLUGIN "TESTLIBS" "" ${ARGN})

  SET(name ${${type}_${data}_prefix}-${plugname})
  CREATE_PLUGIN_COMMON(${name} ${plugname}.cc)
  CREATE_PLUGIN_MODULE(${name} "${libs}" "${PLUGIN_TEST_ROOT}/${plugin_build_path}")
  CREATE_PLUGIN_TEST(${name} test_${plugname}.cc "${libs}" TESTLIBS "${PLUGIN_TESTLIBS}")
  INSTALL(TARGETS ${name} LIBRARY DESTINATION ${install_path})
ENDMACRO(PLUGIN_WITH_TEST_AND_PREFIX2 type data plugname libs)


MACRO(PLUGINGROUP_WITH_PREFIX2 type data plugins libs)
  PARSE_ARGUMENTS(PLUGIN "TESTLIBS" "" ${ARGN})
  FOREACH(p ${plugins})
    PLUGIN_WITH_PREFIX2(${type} ${data} ${p} "${libs}"  TESTLIBS "${PLUGIN_TESTLIBS}")
  ENDFOREACH(p)
ENDMACRO(PLUGINGROUP_WITH_PREFIX2 type data plugins libs)

MACRO(PLUGINGROUP_WITH_TEST_AND_PREFIX2 type data plugins libs)
  PARSE_ARGUMENTS(PLUGIN "TESTLIBS" "" ${ARGN})
  FOREACH(p ${plugins})
    PLUGIN_WITH_TEST_AND_PREFIX2(${type} ${data} ${p} "${libs}"  TESTLIBS "${PLUGIN_TESTLIBS}")
  ENDFOREACH(p)
ENDMACRO(PLUGINGROUP_WITH_TEST_AND_PREFIX2)


MACRO(PLUGIN_WITH_TEST_MULTISOURCE name type data src libs) 

  PARSE_ARGUMENTS(PLUGIN "TESTLIBS" "" ${ARGN})
  TEST_PREFIX(${type} ${data})
  SET(install_path ${${type}_${data}_dir})
  SET(plugin_build_path ${${type}_${data}_binary})
  SET(plugname ${${type}_${data}_prefix}-${name})

  # create common library 
  ADD_LIBRARY(${plugname}-common OBJECT ${src})
  IF(NOT WIN32)
	set_source_files_properties(${src}  PROPERTIES COMPILE_FLAGS "-fPIC")
	set_target_properties(${plugname}-common
          PROPERTIES
          COMPILE_FLAGS -DVSTREAM_DOMAIN='"${plugname}"') 
  ENDIF(NOT WIN32)

  # create module 
  ADD_LIBRARY(${plugname} MODULE $<TARGET_OBJECTS:${plugname}-common>)
  SET_TARGET_PROPERTIES(${plugname} PROPERTIES
    PREFIX "" SUFFIX ${PLUGSUFFIX}
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_TEST_ROOT}/${plugin_build_path}")
  IF(NOT WIN32)
#    SET_TARGET_PROPERTIES(${plugname} PROPERTIES LINK_FLAGS "-Wl,--no-gc-sections -Wl,--undefined,get_plugin_interface")
  ENDIF(NOT WIN32)
  TARGET_LINK_LIBRARIES(${plugname} ${libs})
  ADD_DEPENDENCIES(plugin_test_links ${plugname})
  
  # create tests
  IF(MIA_ENABLE_TESTING)
    PARSE_ARGUMENTS(PLUGIN "TESTLIBS" "" ${ARGN})

    add_executable(test-${plugname} $<TARGET_OBJECTS:${plugname}-common> test_${name}.cc)
    IF(NOT WIN32)
      set_target_properties(test-${plugname} PROPERTIES
        COMPILE_FLAGS -DVSTREAM_DOMAIN='"${plugname}"'
        COMPILE_FLAGS -DBOOST_TEST_DYN_LINK)
    ELSE(NOT WIN32)
      set_target_properties(test-${plugname} PROPERTIES
        COMPILE_FLAGS -DBOOST_TEST_DYN_LINK)
    ENDIF(NOT WIN32)
    target_link_libraries(test-${plugname} ${libs})
    target_link_libraries(test-${plugname} ${BOOST_UNITTEST} "${PLUGIN_TESTLIBS}")
    add_test(${plugname} test-${plugname})
  endif()
  INSTALL(TARGETS ${plugname} LIBRARY DESTINATION ${install_path})
ENDMACRO(PLUGIN_WITH_TEST_MULTISOURCE) 

