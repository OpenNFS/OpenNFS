set(G3_SHARED_LIB OFF CACHE BOOL "Compile g3log as static library")
set(ADD_FATAL_EXAMPLE OFF CACHE BOOL "Don't bother compiling invalid code in g3log")
add_subdirectory("${CMAKE_SOURCE_DIR}/lib/g3log")
