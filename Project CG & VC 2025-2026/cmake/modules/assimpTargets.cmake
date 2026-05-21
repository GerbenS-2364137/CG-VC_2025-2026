find_path(ASSIMP_INCLUDE_DIR assimp/Importer.hpp)
find_library(ASSIMP_LIBRARY assimp)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(assimp DEFAULT_MSG ASSIMP_LIBRARY ASSIMP_INCLUDE_DIR)

if(assimp_FOUND)
  add_library(assimp::assimp UNKNOWN IMPORTED)
  set_target_properties(assimp::assimp PROPERTIES
          IMPORTED_LOCATION "${ASSIMP_LIBRARY}"
          INTERFACE_INCLUDE_DIRECTORIES "${ASSIMP_INCLUDE_DIR}"
  )
endif()