SET(fluid2d_SOURCES main.cc vfluid.cc elast.cc helpers.cc)

ADD_EXECUTABLE(mia-fluid2d ${fluid2d_SOURCES})
TARGET_LINK_LIBRARIES(mia-fluid2d mia2d)
INSTALL(TARGETS mia-fluid2d RUNTIME DESTINATION "bin")

