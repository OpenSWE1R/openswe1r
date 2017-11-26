# Exports:
#  LIBUNICORN_FOUND
#  LIBUNICORN_INCLUDE_DIR
#  LIBUNICORN_LIBRARY
# Hints:
#  LIBUNICORN_LIBRARY_DIR

find_path(LIBUNICORN_INCLUDE_DIR
          unicorn/unicorn.h)

find_library(LIBUNICORN_LIBRARY
             NAMES unicorn
             HINTS "${LIBUNICORN_LIBRARY_DIR}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(unicorn DEFAULT_MSG
                                  LIBUNICORN_LIBRARY LIBUNICORN_INCLUDE_DIR)
mark_as_advanced(LIBUNICORN_INCLUDE_DIR LIBUNICORN_LIBRARY)
