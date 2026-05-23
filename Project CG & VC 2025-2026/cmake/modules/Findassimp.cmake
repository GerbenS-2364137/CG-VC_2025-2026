set(ASSIMP_ROOT "${CMAKE_SOURCE_DIR}/external/assimp")

find_path(ASSIMP_INCLUDE_DIR
        NAMES assimp/Importer.hpp
        PATHS
        "${ASSIMP_ROOT}/include"
        /usr/include
)

find_library(ASSIMP_LIBRARY
        NAMES assimp
        PATHS
        "${ASSIMP_ROOT}/lib"
        /usr/lib/x86_64-linux-gnu
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(assimp DEFAULT_MSG ASSIMP_LIBRARY ASSIMP_INCLUDE_DIR)

if(assimp_FOUND)
  add_library(assimp::assimp UNKNOWN IMPORTED)
  set_target_properties(assimp::assimp PROPERTIES
          IMPORTED_LOCATION "${ASSIMP_LIBRARY}"
          INTERFACE_INCLUDE_DIRECTORIES "${ASSIMP_INCLUDE_DIR}"
  )
endif()