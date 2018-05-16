cmake_minimum_required(VERSION 3.1)
project(OpenSWE1R)

option(USE_VM "Use hardware virtualization backend for Unicorn-Engine" OFF)


set(CMAKE_C_STANDARD 11)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

find_package(ENet)
find_package(Unicorn REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)
find_package(OpenAL REQUIRED)

if(ENET_FOUND)
  include_directories(SYSTEM
    ${ENET_INCLUDE_DIR}
  )
endif()

include_directories(SYSTEM
  ${LIBUNICORN_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIR}
  ${GLEW_INCLUDE_DIRS}
  ${SDL2_INCLUDE_DIR}
  ${OPENAL_INCLUDE_DIR}
)

add_executable(openswe1r
  main.c
  emulation.c
  export.c
  shader.c

  dll/kernel32.c

  com/a3d.c
  com/dplay.c
)

# Determine version information
if(CMAKE_SIZEOF_VOID_P LESS 8)
  set(APP_BUILD_CONFIG x86)
else()
  set(APP_BUILD_CONFIG x64)
endif()
if(WIN32)
  string(APPEND APP_BUILD_CONFIG "-Windows")
  if(MSVC)
    string(APPEND APP_BUILD_CONFIG "-MSVC")
  elseif(MSYS)
    string(APPEND APP_BUILD_CONFIG "-MSYS")
  endif()
elseif(APPLE)
  string(APPEND APP_BUILD_CONFIG "-macOS")
else()
  string(APPEND APP_BUILD_CONFIG "-Linux")
endif()

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/app_version.h
  PRE_BUILD
  COMMAND
    ${CMAKE_COMMAND}
    -D APP_VERSION_DIR=${CMAKE_BINARY_DIR}
    -D APP_BUILD_CONFIG=${APP_BUILD_CONFIG}
    -P cmake/create_app_version.cmake
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
target_sources(openswe1r PUBLIC ${CMAKE_BINARY_DIR}/app_version.h)
target_include_directories(openswe1r PRIVATE ${CMAKE_BINARY_DIR})

if(USE_VM)
  target_compile_definitions(openswe1r PUBLIC -DUC_KVM)
  target_sources(openswe1r PUBLIC
    uc_kvm.c
  )
else()
  target_link_libraries(openswe1r
    ${LIBUNICORN_LIBRARY}
  )
endif()

if(ENET_FOUND)
  target_compile_definitions(openswe1r PUBLIC -DDPLAY_ENET)
  target_link_libraries(openswe1r
    ${ENET_LIBRARIES}
  )
endif()

target_link_libraries(openswe1r
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${SDL2_LIBRARY}
  ${OPENAL_LIBRARY}
)

if(MSVC)
  # Silence MSVC CRT security warnings
  target_compile_definitions(openswe1r PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
