# This script will determine the current version based on git revision
# or tag and create information files to be consumed during compile
# and package time

include(${CMAKE_SOURCE_DIR}/cmake/determine_app_version.cmake)

# Write app version to header if changed
set(APP_VERSION_FILE_NEEDS_UPDATE TRUE)
set(APP_VERSION_FILE "${APP_VERSION_DIR}/app_version.h")
if(EXISTS ${APP_VERSION_FILE})
  file(READ ${APP_VERSION_FILE} VERSION_H_CONTENTS)
  string(REGEX MATCH "APP_VERSION_STRING[ \t]+\"(.+)\""
    FILE_VERSION ${VERSION_H_CONTENTS})
  string(REGEX MATCH "\"(.+)\""
    FILE_VERSION ${FILE_VERSION})
  string(REGEX MATCH "[^\"]+"
    FILE_VERSION ${FILE_VERSION})
  if(FILE_VERSION STREQUAL APP_VERSION)
    set(APP_VERSION_FILE_NEEDS_UPDATE FALSE)
  endif()
endif()

if(APP_VERSION_FILE_NEEDS_UPDATE)
  configure_file("${CMAKE_SOURCE_DIR}/app_version.h.in" ${APP_VERSION_FILE} @ONLY)
endif()
