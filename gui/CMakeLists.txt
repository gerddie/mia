

FIND_PACKAGE(wxWidgets COMPONENTS base core  gl)

MACRO(ADD_WXEXE name sources libs)
  ADD_EXECUTABLE(miagui-${name} ${sources})
  TARGET_LINK_LIBRARIES(miagui-${name} ${wxWidgets_LIBRARIES}  ${libs} ${BASELIBS})
  SET_TARGET_PROPERTIES(miagui-${name}  PROPERTIES COMPILE_FLAGS  ${wxWidgets_CXX_FLAGS})
  IF(UNIX)
    # no warnings about breaking aliasing rules - wx headers issue a lot of them 
    SET_TARGET_PROPERTIES(miagui-${name}  PROPERTIES COMPILE_FLAGS -Wno-strict-aliasing)
  ENDIF(UNIX)
  INSTALL(TARGETS miagui-${name} RUNTIME DESTINATION "bin")
ENDMACRO(ADD_WXEXE)

IF (wxWidgets_FOUND) 
  INCLUDE(${wxWidgets_USE_FILE})

  SET(MYCARD_SOURCE 
    myocard.cc
    serieswindow.cc
    curvedisplay.cc
    )
  ADD_WXEXE(myocard "${MYCARD_SOURCE}" mia2d)


ENDIF (wxWidgets_FOUND) 