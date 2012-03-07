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

INCLUDE (${CMAKE_ROOT}/Modules/FindDoxygen.cmake)

ADD_CUSTOM_TARGET(docs echo "creating documentation")
add_dependencies(doc docs)  

ADD_CUSTOM_TARGET(xmldocs)
ADD_CUSTOM_TARGET(manpages)

ADD_CUSTOM_TARGET(manpath mkdir -p man)

add_dependencies(manpages xmldocs)
add_dependencies(docs manpages)  


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

ENDIF (DOXYGEN_FOUND)


# handle the linking of some files to run the 
# 

ADD_CUSTOM_TARGET(userref_outdir mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/userref)
ADD_CUSTOM_TARGET(userref_css cp  ${CMAKE_CURRENT_SOURCE_DIR}/progref.css ${CMAKE_CURRENT_BINARY_DIR}/userref/progref.css)
ADD_CUSTOM_TARGET(userref_create_xml ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/miaxml2sgml.py) 
ADD_CUSTOM_TARGET(userref_link_source ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/progref.xml ${CMAKE_CURRENT_BINARY_DIR}/) 
ADD_CUSTOM_TARGET(userref ${CMAKE_CURRENT_SOURCE_DIR}/create_chunked_html.sh) 

add_dependencies(userref_create_xml xmldocs)

add_dependencies(userref userref_create_xml) 
add_dependencies(userref userref_link_source)
add_dependencies(userref userref_css)
add_dependencies(userref userref_outdir)
add_dependencies(docs userref)  


ADD_CUSTOM_TARGET(install-manpages 
  mkdir -p "${CMAKE_INSTALL_PREFIX}/man/man1"
  COMMAND cp -r man/* "${CMAKE_INSTALL_PREFIX}/man/man1"
  DEPENDS manpages)

add_dependencies(install-doc install-manpages)
