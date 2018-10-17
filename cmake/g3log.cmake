set(G3LOG_NAME g3log)

# Specifically pick the static version since the build generates both static and dynamic libraries
set(G3LOGGER_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/lib/${G3LOG_NAME}/build/libg3logger.a")

set(GENERATOR "CodeBlocks - MinGW Makefiles")

add_definitions()

if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/lib/${G3LOG_NAME}/build)
    message("Configuring G3log")
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib/${G3LOG_NAME}/build)
    execute_process(COMMAND ${CMAKE_COMMAND}
            "-G${GENERATOR}"
            "-DCMAKE_BUILD_TYPE=Release"
            "-DG3_SHARED_LIB=OFF"
            "-H${DEP_ROOT_DIR}/${G3LOG_NAME}"
            "-B${CMAKE_CURRENT_BINARY_DIR}/lib/${G3LOG_NAME}/build"
            RESULT_VARIABLE
            G3LOG_CONFIGURE)
    if(NOT G3LOG_CONFIGURE EQUAL 0)
        message(FATAL_ERROR "G3log Configure failed!")
    endif()

    #[[ TODO: Move this from project-gen time to build time, as a target ]]
    message("Building G3log")
    execute_process(COMMAND ${CMAKE_COMMAND} --build
            "${CMAKE_CURRENT_BINARY_DIR}/lib/${G3LOG_NAME}/build"
            RESULT_VARIABLE
            G3LOG_BUILD)
    if(NOT G3LOG_BUILD EQUAL 0)
        message(FATAL_ERROR "G3log build failed!")
    endif()
endif()
