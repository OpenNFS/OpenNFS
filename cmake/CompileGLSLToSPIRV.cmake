function(CompileGLSLToSpirV TARGET_NAME INPUT_DIR OUTPUT_DIR)
    if (WIN32)
        if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "AMD64")
            set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin/glslangValidator.exe")
        else ()
            set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin32/glslangValidator.exe")
        endif ()
    else ()
        set(GLSL_VALIDATOR "glslangValidator")
    endif ()

    file(GLOB_RECURSE GLSL_SOURCE_FILES
            "${INPUT_DIR}/*.frag"
            "${INPUT_DIR}/*.vert"
            )

    message("The following GLSL shaders will be compiled to SPIRV:")
    foreach (GLSL ${GLSL_SOURCE_FILES})
        get_filename_component(FILE_NAME ${GLSL} NAME)
        message("- " ${GLSL})
        set(SPIRV "${OUTPUT_DIR}/${FILE_NAME}.spv")
        add_custom_command(
                OUTPUT ${SPIRV}
                COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_DIR}"
                COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
                DEPENDS ${GLSL})
        list(APPEND SPIRV_BINARY_FILES ${SPIRV})
    endforeach (GLSL)

    add_custom_target(
            Shaders
            DEPENDS ${SPIRV_BINARY_FILES}
    )

    add_dependencies(${TARGET_NAME} Shaders)
endfunction()