SET(fluid3d_SOURCES 
  eqn_solver.cc
  main.cc
  sor_solver.cc
  vfluid.cc
  )

ADD_EXECUTABLE(mia-fluid3d ${fluid3d_SOURCES})
TARGET_LINK_LIBRARIES(mia-fluid3d mia3d)
INSTALL(TARGETS mia-fluid3d RUNTIME DESTINATION "bin")




