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


INCLUDE (${CMAKE_ROOT}/Modules/FindDoxygen.cmake)

ADD_CUSTOM_TARGET(docs echo "creating documentation")
add_dependencies(doc docs)  

IF(CREATE_USERDOC) 

  add_dependencies(docs manpages)
  IF(ALWAYS_CREATE_DOC) 
    INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/man/ DESTINATION share/man/man1/)
  ENDIF(ALWAYS_CREATE_DOC) 

  ##################################################################
  #  for html userref xsltproc and the style sheet is needed
  #
  find_program(XSLTPROC xsltproc)
  IF(XSLTPROC)
    FIND_PATH(HTML_CHUNK_FILEPATH chunk.xsl
      PATHS 
      /usr/share/sgml/docbook/xsl-stylesheets/html/
      /usr/share/xml/docbook/stylesheet/docbook-xsl/html/
      DOC "style sheet for the creation of the user reference"
      )
  ELSE (XSLTPROC)
    IF(STRICT_DEPENDECIES)
      MESSAGE(ERROR ": xsltproc not found and STRICT_DEPENDECIES set")
    ENDIF(STRICT_DEPENDECIES)
  ENDIF(XSLTPROC)
  
  IF(HTML_CHUNK_FILEPATH) 
    #
    # handle the linking of some files to run the 
    # 
    ADD_CUSTOM_TARGET(userref_outdir mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/userref)
    ADD_CUSTOM_TARGET(userref_css 
      cp  ${CMAKE_CURRENT_SOURCE_DIR}/progref.css 
      ${CMAKE_CURRENT_BINARY_DIR}/userref/progref.css DEPENDS userref_outdir)

    ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/progref.xml
      COMMAND ln ARGS -sf ${CMAKE_CURRENT_SOURCE_DIR}/progref.xml ${CMAKE_CURRENT_BINARY_DIR}/progref.xml) 

    ADD_CUSTOM_TARGET(progref_link DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/progref.xml)
      
    ADD_CUSTOM_COMMAND(OUTPUT  "userref.stamp"
      COMMAND  
      ${XSLTPROC} ARGS 
      --param chunk.section.depth 3 
      --param chunk.first.sections 1 
      --param use.id.as.filename 1 
      --stringparam html.stylesheet  progref.css 
      --output "userref/" 
      ${HTML_CHUNK_FILEPATH}/chunk.xsl progref.xml
      COMMAND touch ARGS userref.stamp
      DEPENDS process_xml userref_css userref_outdir progref_link ${CMAKE_CURRENT_BINARY_DIR}/program.xml
      ) 
    ADD_CUSTOM_TARGET(userref DEPENDS userref.stamp)
    add_dependencies(docs userref)
    
    ####################################################
    #
    # installation of the user reference 
    IF(ALWAYS_CREATE_DOC) 
      INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/userref DESTINATION ${DOC_INSTALL_PATH}/)
      CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/mia2-doc.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/mia2-doc.desktop)
      INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/mia2-doc.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
    ENDIF(ALWAYS_CREATE_DOC)     
  ENDIF(HTML_CHUNK_FILEPATH) 
  
ENDIF(CREATE_USERDOC)

IF (DOXYGEN_FOUND)
  ADD_CUSTOM_TARGET(LibraryDoc echo "creating user library documentation")
  ADD_DEPENDENCIES(docs LibraryDoc)
  
  IF(DOXYGEN_DOT_FOUND)
    SET(HAVE_DOT YES)
  ENDIF(DOXYGEN_DOT_FOUND)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/reference.dox.cmake 
    ${CMAKE_CURRENT_BINARY_DIR}/reference.dox)
  
  ADD_CUSTOM_COMMAND(SOURCE COMMAND ${DOXYGEN_EXECUTABLE}
    ARGS  "${CMAKE_CURRENT_BINARY_DIR}/reference.dox"
    TARGET LibraryDoc
    OUTPUTS libdoc)
  
  IF(ALWAYS_CREATE_DOC) 
    INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libdoc/html/ DESTINATION ${DOC_INSTALL_PATH}/libdoc/)
    CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/libmia2-doc.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/libmia2-doc.desktop)
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/libmia2-doc.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
  ENDIF(ALWAYS_CREATE_DOC)     
 
ENDIF (DOXYGEN_FOUND)



