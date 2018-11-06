# This script will determine the current version based on git revision or tag

# Determine version number

# When AppVeyor builds a tag use that tags name as version
if("$ENV{APPVEYOR_REPO_TAG}" STREQUAL "true")
  set(APP_VERSION $ENV{APPVEYOR_REPO_TAG_NAME})
else()
  # Default to git revision
  execute_process(
    COMMAND git show -s --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_REVISION
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(APP_VERSION g${GIT_REVISION})
endif()

# Determine if the current version is "dirty"
execute_process(
  COMMAND git status --porcelain
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_STATUS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(GIT_STATUS)
  string(APPEND APP_VERSION "-dirty")
endif()

if(APP_BUILD_CONFIG)
  string(APPEND APP_VERSION "-${APP_BUILD_CONFIG}")
endif()
