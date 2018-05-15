# Configure CPack variables before including CPack

set(CPACK_PACKAGE_VENDOR OpenSWE1R)
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE.txt)

if(NOT CPACK_GENERATOR)
  # Set default list of supported CPack generators if not specified by the user
  if(WIN32 OR APPLE)
    set(CPACK_GENERATOR "ZIP")
  else()
    set(CPACK_GENERATOR "TGZ")
  endif()
endif()

include(cmake/determine_app_version.cmake)
set(CPACK_PACKAGE_VERSION ${APP_VERSION})
set(CPACK_PACKAGE_FILE_NAME OpenSWE1R-${APP_VERSION})

include(CPack)

# Install required files
set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ".")

# Install main binaries
install(TARGETS openswe1r
  RUNTIME DESTINATION ${CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION}
)

# Install documentation
install(FILES README.md LICENSE.txt DESTINATION ${CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION})

# Install additional binary modules
if(MSVC)
  # Install required runtime libraries
  include(InstallRequiredSystemLibraries)

  # vcpkg will automatically move the required binaries to the build folder
  foreach(lib OpenAL32 unicorn SDL2)
    install(FILES $<TARGET_FILE_DIR:openswe1r>/${lib}.dll
      DESTINATION ${CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION}
  )
  endforeach()
  install(FILES $<TARGET_FILE_DIR:openswe1r>/glew32$<$<CONFIG:Debug>:d>.dll
    DESTINATION ${CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION}
  )
endif()
