INCLUDE (${CMAKE_ROOT}/Modules/FindLATEX.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/FindDoxygen.cmake)


ADD_CUSTOM_TARGET(doc echo "creating documentation")

IF (DOXYGEN_FOUND)

   ADD_CUSTOM_TARGET(LibraryDoc echo "creating user library documentation")
   ADD_DEPENDENCIES(doc LibraryDoc)

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




IF (PDFLATEX_COMPILER)	
  FIND_PACKAGE(PythonInterp)
   ADD_CUSTOM_TARGET(UserGuide echo "creating user user guide")
   ADD_DEPENDENCIES(doc UserGuide)

   SET(GUIDE_SRC 
          nonfloat.sty
          userguide.tex userguide.bib
	  definitions.tex
	  matlab.tex
	  idl.tex
	  running.tex
          programs.tex 
   )
    
   CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/version.tex.in 
                    ${CMAKE_CURRENT_BINARY_DIR}/version.tex)


   ADD_CUSTOM_TARGET(user_guide_plugins  echo "creating plug-in documentation")
   if (PYTHONINTERP_FOUND)
     ADD_CUSTOM_COMMAND(SOURCE COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/extract_docu.py 

       ARGS  ${CMAKE_CURRENT_SOURCE_DIR}/../mia plugins_new.tex
       TARGET user_guide_plugins
       OUTPUTS plugins_new.tex)
   else(PYTHONINTERP_FOUND)
     ADD_CUSTOM_COMMAND(SOURCE COMMAND echo "Needs python to create documentation" >plugins_new.tex
       TARGET user_guide_plugins
       OUTPUTS plugins_new.tex)
   endif(PYTHONINTERP_FOUND)
   
   ADD_CUSTOM_TARGET(user_guide_aux  echo "creating userguide step 1")

   ADD_DEPENDENCIES(user_guide_aux user_guide_plugins)
   ADD_CUSTOM_TARGET(user_guide_bbl  echo "creating userguide step 2")
   ADD_DEPENDENCIES(user_guide_bbl user_guide_aux)

   ADD_CUSTOM_TARGET(user_guide_3  echo "creating userguide step 3")
   ADD_DEPENDENCIES(user_guide_3 user_guide_bbl)
   ADD_DEPENDENCIES(UserGuide user_guide_3)

   IF (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
         ADD_DEPENDENCIES(user_guide_aux ${GUIDE_SRC} )
   ELSE (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
      IF(WIN32)
	
      ELSE (WIN32)
	FOREACH(f ${GUIDE_SRC})
	   ADD_CUSTOM_TARGET(guide_src${f} ln -sf  ${CMAKE_CURRENT_SOURCE_DIR}/${f} ${CMAKE_CURRENT_BINARY_DIR}/${f})
	   ADD_DEPENDENCIES(user_guide_aux guide_src${f})
        ENDFOREACH(f)
      ENDIF(WIN32)
   ENDIF (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})

   MACRO(pdfcompile file target out)
      ADD_CUSTOM_COMMAND(SOURCE COMMAND ${PDFLATEX_COMPILER}
          ARGS  ${file}
          TARGET ${target}
	  OUTPUTS ${out})
   ENDMACRO(pdfcompile)

   pdfcompile(userguide.tex user_guide_aux userguide.aux)
   pdfcompile(userguide.tex user_guide_3 userguide.aux)
   pdfcompile(userguide.tex UserGuide userguide.pdf)

   ADD_CUSTOM_COMMAND(SOURCE COMMAND ${BIBTEX_COMPILER}
          ARGS  userguide
          TARGET user_guide_bbl
	  OUTPUTS userguide.bbl)

ENDIF (PDFLATEX_COMPILER)
