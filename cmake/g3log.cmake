set(G3LOG_NAME g3log)

set(G3LOGGER_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/lib/${G3LOG_NAME}/build")

# Specifically pick the static version since the build generates both static and dynamic libraries
if (MSVC)
    set(G3LOGGER_LIBRARIES "${G3LOGGER_BUILD_DIR}/libg3logger.lib")
else ()
    set(G3LOGGER_LIBRARIES "${G3LOGGER_BUILD_DIR}/libg3logger.a")
endif ()

#[[On Windows, the generator for g3logger seems to change to MSVC automatically]]
if (MINGW)
    set(GENERATOR "CodeBlocks - MinGW Makefiles")
elseif (MSVC)
    set(GENERATOR "Visual Studio 15 2017")
else ()
    set(GENERATOR ${CMAKE_GENERATOR})
endif ()

set(SH_NOT_FOUND "CMAKE_SH-NOTFOUND")

if (NOT EXISTS G3LOGGER_BUILD_DIR)
    message("Configuring G3log")
    file(MAKE_DIRECTORY ${G3LOGGER_BUILD_DIR})
    execute_process(COMMAND ${CMAKE_COMMAND}
            "-G${GENERATOR}"
            "-DCMAKE_SH=${SH_NOT_FOUND}"
            "-DCMAKE_BUILD_TYPE=Release"
            "-DG3_SHARED_LIB=OFF"
            "-H${DEP_ROOT_DIR}/${G3LOG_NAME}"
            "-B${G3LOGGER_BUILD_DIR}"
            RESULT_VARIABLE
            G3LOG_CONFIGURE)
    if (NOT G3LOG_CONFIGURE EQUAL 0)
        file(REMOVE ${G3LOGGER_BUILD_DIR})
        message(FATAL_ERROR "G3log Configure failed!")
        return()
    else ()
        add_custom_target(g3logger COMMAND ${CMAKE_COMMAND} --build ${G3LOGGER_BUILD_DIR})
    endif ()
endif ()
