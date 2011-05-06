INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MIA - Medical Image Analysis")
SET(CPACK_PACKAGE_VENDOR "${VENDOR}")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
SET(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_PACKAGE_VERSION_MAJOR ${MAJOR_VERSION})
SET(CPACK_PACKAGE_VERSION_MINOR ${MINOR_VERSION})
SET(CPACK_PACKAGE_VERSION_PATCH ${MICRO_VERSION})
SET(CPACK_PACKAGE_CONTACT "gw.fossdev@gmail.com")
IF(WIN32 AND NOT UNIX)
  SET(CPACK_PACKAGE_INSTALL_DIRECTORY "Mia")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\mia.sourceforge.net")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\mia.sourceforge.net")
  SET(CPACK_NSIS_CONTACT "gw.fossdev@gmail.com")
  SET(CPACK_NSIS_MODIFY_PATH ON)
  SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY ${MIA_NAME})
  SET(CPACK_GENERATOR "NSIS")
  
  GET_FILENAME_COMPONENT( MSVC_REDIST_EXE
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\S-1-5-18\\Components\\9477E47492EDAAF44936E05ACF0DED92;4E1DAD7D4F54B2B398A9AE271876CEF4]
    PATH)
  IF (MSVC_REDIST_EXE) 
    SET( CPACK_NSIS_EXTRA_INSTALL_COMMANDS ${CPACK_NSIS_EXTRA_INSTALL_COMMANDS}
      " Set output Path\\n  SetOutPath \\\"$INSTDIR\\\\vc9\\\"\\n"
      " Code to install Visual studio redistributable\\n  File \\\"${Temp_vc9_redist}\\\"\\n"
      " Execute silent and wait\\n  ExecWait '\\\"$INSTDIR\\\\vc9\\\\${MSVC_REDIST_EXE}\\\"/q:a /c:\\\"msiexec /i vcredist.msi /qn /l*v %temp%\\\\vcredist_x86.log\\\"' $0\\n"
      " Wait a bit for system to unlock file.\\n  Sleep 1000\\n"
      " Delete file\\n  Delete \\\"$INSTDIR\\\\vc9\\\\${MSVC_REDIST_EXE}\\\"\\n"
      " Reset output Path\\n  SetOutPath \\\"$INSTDIR\\\"\\n"
      " Remove folder\\n  RMDir /r \\\"$INSTDIR\\\\vc9\\\"\\n\\n" )
    
  ELSE(MSVC_REDIST_EXE)
    MESSAGE(ERROR "Praying didn't work, so you have to set 'MSVC_REDIST_EXE' to the vcredist_x86_exe of your Visual Studio  installation.")
  ENDIF(MSVC_REDIST_EXE)
  
ELSE(WIN32 AND NOT UNIX)
  SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${CMAKE_INSTALL_PREFIX}")
  SET(CPACK_SOURCE_GENERATOR "TGZ")
  SET(CPACK_GENERATOR "DEB")
ENDIF(WIN32 AND NOT UNIX)

INCLUDE(CPack)
