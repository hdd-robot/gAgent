#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "gAgent::gagent" for configuration "Release"
set_property(TARGET gAgent::gagent APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(gAgent::gagent PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libgagent.so"
  IMPORTED_SONAME_RELEASE "libgagent.so"
  )

list(APPEND _cmake_import_check_targets gAgent::gagent )
list(APPEND _cmake_import_check_files_for_gAgent::gagent "${_IMPORT_PREFIX}/lib/libgagent.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
