#
# This file is part of MIA - a toolbox for medical image analysis 
# Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
#
# MIA is free software; you can redistribute it and/or modify
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
# along with this program; if not, see <http://www.gnu.org/licenses/>.
#


#
# This macro is used to prepare the creation of the man pages and 
# interfaces 
# - prefix will be used to set the prefix of the executables and the 
# target directory of the nipype interface 
#
MACRO(MIA_PREPARE_AUTODOC prefix)

  IF(NOT MIA_DOCTOOLS_ROOT)
    SET(MIA_DOCTOOLS_ROOT "${CMAKE_SOURCE_DIR}/doc")
  ENDIF(NOT MIA_DOCTOOLS_ROOT)
  
  OPTION(MIA_CREATE_MANPAGES "Create the man pages for the executables (Required Python and python-lxml)" OFF)
  OPTION(MIA_CREATE_NIPYPE_INTERFACES "Create the nipype interfaces for the executables (Required Python,python-lxml, and nipype)" OFF)
  
  IF(MIA_CREATE_MANPAGES OR MIA_CREATE_NIPYPE)
    
    FIND_PACKAGE(PythonInterp REQUIRED)
    EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c "import lxml"  RESULT_VARIABLE LXML_ERR)
    IF(LXML_ERR) 
      MESSAGE(FATAL "Python found, but no pythonl-xml")
    ENDIF(LXML_ERR)
    
    IF(MIA_CREATE_MANPAGES) 
      ADD_CUSTOM_TARGET(manpages ALL)
    ENDIF()
    
    IF(MIA_CREATE_NIPYPE_INTERFACES)
      file(WRITE ${NIPYPE_INTERFACE_INIT_FILE} "# Automatically generated file, do not edit\n")

      STRING(COMPARE EQUAL "${CMAKE_INSTALL_PREFIX}" "/usr" INSTALLROOT_IS_USER)
      
      IF(INSTALLROOT_IS_USER)
	EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c "from distutils.sysconfig import get_python_lib\nimport sys\nsys.stdout.write(get_python_lib())"
	  RESULT_VARIABLE SITEPACKGE_ERR
	  OUTPUT_VARIABLE SITEPACKGE_BASE_PATH)
      ELSE()
	EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c "import site\nimport sys\nsys.stdout.write(site.getusersitepackages())"
	  RESULT_VARIABLE SITEPACKGE_ERR
	  OUTPUT_VARIABLE SITEPACKGE_BASE_PATH)
      ENDIF()

      IF(SITEPACKGE_ERR) 
        MESSAGE(FATAL "Something went wrong identifying the nipype installation loaction") 
      ENDIF()

      SET(NIPYPE_INTERFACE_DIR "${SITEPACKGE_BASE_PATH}/${prefix}/nipype/interfaces/")
      
      
      
      MESSAGE(STATUS "Will create nipype interfaces and install to " ${NIPYPE_INTERFACE_DIR}) 
      
      ADD_CUSTOM_TARGET(nipypeinterfaces ALL)
      INSTALL(FILES ${NIPYPE_INTERFACE_INIT_FILE} DESTINATION ${NIPYPE_INTERFACE_DIR})
    ENDIF()
  ENDIF()

  # install empty init files 
  INSTALL(FILES ${MIA_DOCTOOLS_ROOT}/__init__.py DESTINATION ${SITEPACKGE_BASE_PATH}/${prefix})
  INSTALL(FILES ${MIA_DOCTOOLS_ROOT}/__init__.py DESTINATION ${SITEPACKGE_BASE_PATH}/${prefix}/nipype)
  
ENDMACRO(MIA_PREPARE_AUTODOC) 


#
# INTERNAL USE 
# This macro runs the program to create the XML program descrition 
# that is used to create documentation and interfaced 
# TODO: add possible plug-ins that come directly from the software package 
#
MACRO(MIA_CREATE_EXE_XML_HELP prefix name)
  ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_BINARY_DIR}/doc/${prefix}-${name}.xml
    COMMAND ./${prefix}-${name} --help-xml ${CMAKE_BINARY_DIR}/doc/${prefix}-${name}.xml
    DEPENDS ${prefix}-${name})
    
  ADD_CUSTOM_TARGET(${prefix}-${name}-xml DEPENDS ${CMAKE_BINARY_DIR}/doc/${prefix}-${name}.xml)
  ADD_DEPENDENCIES(xmldoc ${prefix}-${name}-xml)
ENDMACRO(MIA_CREATE_EXE_XML_HELP)

#
# INTERNAL USE 
# Create the nipype interface from the xml doc description 
# and add it to the install target 
#

MACRO(MIA_CREATE_NIPYPE_FROM_XML prefix name)
  STRING(REPLACE "-" "_" PythonName ${name})
  
  SET(${prefix}-${name}-nipype-interface ${CMAKE_CURRENT_BINARY_DIR}/${prefix}_${PythonName}.py)
  
  ADD_CUSTOM_COMMAND(OUTPUT ${${prefix}-${name}-nipype-interface} 
    COMMAND ${PYTHON_EXECUTABLE} ${MIA_DOCTOOLS_ROOT}/miaxml2nipype.py -i ${CMAKE_BINARY_DIR}/doc/${prefix}-${name}.xml -o ${${prefix}-${name}-nipype-interface}
    MAIN_DEPENDENCY ${CMAKE_BINARY_DIR}/doc/${prefix}-${name}.xml)

  FILE(APPEND ${NIPYPE_INTERFACE_INIT_FILE} "from .${prefix}_${PythonName} import ${prefix}_${PythonName}\n")
  
  ADD_CUSTOM_TARGET(${prefix}-${name}-nipype DEPENDS ${${prefix}-${name}-nipype-interface})
  ADD_DEPENDENCIES(nipypeinterfaces ${prefix}-${name}-nipype)
  
  INSTALL(FILES ${${prefix}-${name}-nipype-interface} DESTINATION ${NIPYPE_INTERFACE_DIR})
ENDMACRO(MIA_CREATE_NIPYPE_FROM_XML)

#
# INTERNAL USE 
# Create the man page from the xml doc description 
# and add it to the install target 
#
MACRO(MIA_CREATE_MANPAGE_FROM_XML prefix name)
  SET(${prefix}-${name}-manfile ${CMAKE_CURRENT_BINARY_DIR}/${prefix}-${name}.1)
  ADD_CUSTOM_COMMAND(OUTPUT   ${${prefix}-${name}-manfile}
    COMMAND ${PYTHON_EXECUTABLE} ${MIA_DOCTOOLS_ROOT}/miaxml2man.py ${CMAKE_BINARY_DIR}/doc/${prefix}-${name}.xml >${${prefix}-${name}-manfile}
      MAIN_DEPENDENCY ${CMAKE_BINARY_DIR}/doc/${prefix}-${name}.xml
      )
    ADD_CUSTOM_TARGET(${prefix}-${name}-man DEPENDS ${${prefix}-${name}-manfile})
    add_dependencies(manpages ${prefix}-${name}-man)
    INSTALL(FILES ${${prefix}-${name}-manfile} DESTINATION "share/man/man1")
ENDMACRO(MIA_CREATE_MANPAGE_FROM_XML)


#
# INTERNAL USE 
# Run the necessary steps to create the documentation 
# and nipype interfaces (if enabled)
#

MACRO(MIA_EXE_CREATE_DOCU_AND_INTERFACE prefix name)
  MIA_CREATE_EXE_XML_HELP(${prefix} ${name})
  
  IF(MIA_CREATE_MANPAGES) 
    MIA_CREATE_MANPAGE_FROM_XML(${prefix} ${name})
  ENDIF(MIA_CREATE_MANPAGES)

  IF(MIA_CREATE_NIPYPE_INTERFACES)
    MIA_CREATE_NIPYPE_FROM_XML(${prefix} ${name})
  ENDIF(MIA_CREATE_NIPYPE_INTERFACES)
  
ENDMACRO(MIA_EXE_CREATE_DOCU_AND_INTERFACE)

#
# Create an executable and its documentation and nipype interface 
# Parameters: 
#   prefix: napespace prefix of the exe
#   name:   functional name of the exe 
#   libraries: libraries to link this exe against 
#
MACRO(MIA_DEFEXE prefix name libraries) 
  ADD_EXECUTABLE(${prefix}-${name} ${name}.cc)
  TARGET_LINK_LIBRARIES(${prefix}-${name} ${libraries})
  INSTALL(TARGETS ${prefix}-${name} RUNTIME DESTINATION "bin")
  MIA_EXE_CREATE_DOCU_AND_INTERFACE(${prefix}  ${name})

ENDMACRO(MIA_DEFEXE)

#
# Create an executable and its documentation and nipype interface 
# that also provides a selftest 
# 
# Parameters: 
#   prefix: napespace prefix of the exe
#   name:   functional name of the exe 
#   libraries: libraries to link this exe against 
#
MACRO(MIA_DEFEXE_WITH_TEST prefix name deps) 

  MIA_DEFEXE(${prefix} ${name} ${deps})
  ADD_TEST(${name} ${prefix}-${name} --selftest)

ENDMACRO(MIA_DEFEXE_WITH_TEST)
