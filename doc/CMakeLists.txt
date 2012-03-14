#
# Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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

INCLUDE (${CMAKE_ROOT}/Modules/FindDoxygen.cmake)

ADD_CUSTOM_TARGET(docs echo "creating documentation")
add_dependencies(doc docs)  

ADD_CUSTOM_TARGET(xmldoc)

IF(CREATE_USERDOC) 

  ADD_CUSTOM_TARGET(install-manpages 
    mkdir -p $(DESTDIR)/"${CMAKE_INSTALL_PREFIX}/share/man/man1/"
    COMMAND cp man/* $(DESTDIR)/"${CMAKE_INSTALL_PREFIX}/share/man/man1/"
    DEPENDS manpages)

  add_dependencies(docs manpages)    
  add_dependencies(install-doc install-manpages)
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
  ENDIF(XSLTPROC)

  IF(HTML_CHUNK_FILEPATH) 
    #
    # handle the linking of some files to run the 
    # 
    ADD_CUSTOM_TARGET(userref_outdir mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/userref)
    ADD_CUSTOM_TARGET(userref_css 
      cp  ${CMAKE_CURRENT_SOURCE_DIR}/progref.css 
      ${CMAKE_CURRENT_BINARY_DIR}/userref/progref.css DEPENDS userref_outdir)
    ADD_CUSTOM_TARGET(process_xml
      COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/miaxml2sgml.py 
      COMMAND rm -f userref.stamp
      DEPENDS xmldoc
      )
    ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/progref.xml
      COMMAND ln ARGS -sf ${CMAKE_CURRENT_SOURCE_DIR}/progref.xml ${CMAKE_CURRENT_BINARY_DIR}/progref.xml) 
    ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/sections.xml
      COMMAND ln ARGS -sf ${CMAKE_CURRENT_SOURCE_DIR}/sections.xml ${CMAKE_CURRENT_BINARY_DIR}/sections.xml) 

    ADD_CUSTOM_TARGET(progref_link 
      DEPENDS 
      ${CMAKE_CURRENT_BINARY_DIR}/progref.xml 
      ${CMAKE_CURRENT_BINARY_DIR}/sections.xml)
      
    ADD_CUSTOM_COMMAND(OUTPUT  "userref.stamp"
      COMMAND  
      ${XSLTPROC} ARGS 
      --param chunk.section.depth 3 
      --param chunk.first.sections 1 
      --stringparam html.stylesheet  progref.css 
      --output "userref/" 
      ${HTML_CHUNK_FILEPATH}/chunk.xsl progref.xml
      COMMAND touch ARGS userref.stamp
      DEPENDS process_xml userref_css userref_outdir progref_link
      ) 
    ADD_CUSTOM_TARGET(userref DEPENDS userref.stamp)
    add_dependencies(docs userref)  
    
    ####################################################
    #
    # installation of the user reference 
    
    
    ADD_CUSTOM_TARGET(install-userref 
      mkdir -p $(DESTDIR)/"${FULL_DOC_INSTALL_PATH}/userref"
      COMMAND cp -r userref/* $(DESTDIR)/"${FULL_DOC_INSTALL_PATH}/userref"
      DEPENDS userref)
    
    
    add_dependencies(install-doc install-userref)
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
  
  ADD_CUSTOM_TARGET(install-libdoc 
    mkdir -p $(DESTDIR)/"${FULL_DOC_INSTALL_PATH}/libdoc"
    COMMAND cp libdoc/html/* $(DESTDIR)/"${FULL_DOC_INSTALL_PATH}/libdoc"
    DEPENDS LibraryDoc)
  
  add_dependencies(install-doc install-libdoc)
ENDIF (DOXYGEN_FOUND)



