cmake_minimum_required(VERSION 3.20)
include(FetchContent)
project(moltenUI CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(EXE_NAME ${PROJECT_NAME})

find_package(Vulkan REQUIRED)
find_package(glfw3 3.4 REQUIRED)

FetchContent_Declare(fetch_vk_bootstrap GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap GIT_TAG main)
FetchContent_Declare(fetch_vma GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator GIT_TAG master)
FetchContent_MakeAvailable(fetch_vk_bootstrap fetch_vma)

add_executable(${EXE_NAME} 
    src/main.cpp 
    src/init.cpp 
    src/render.cpp 
    src/utils/render.cpp 
    src/shader.cpp
)

target_include_directories(${EXE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(${EXE_NAME} PRIVATE -Wno-nullability-completeness)
target_link_libraries(${EXE_NAME} PRIVATE Vulkan::Vulkan glfw vk-bootstrap::vk-bootstrap VulkanMemoryAllocator)

find_program(GLSLC_EXECUTABLE glslc REQUIRED)
set(SHADER_FILES shaders/rect.vert shaders/rect.frag)

# This directory is where the .exe actually lives
set(SHADER_BIN_DIR "$<TARGET_FILE_DIR:${EXE_NAME}>/shaders")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/shaders_temp") # Ensure temp dir exists

foreach(SHADER ${SHADER_FILES})
    get_filename_component(FILE_NAME ${SHADER} NAME)
    # Output directly to a folder next to the executable
    set(SPIRV_OUT "${CMAKE_CURRENT_BINARY_DIR}/shaders_temp/${FILE_NAME}.spv")
    
    add_custom_command(
        OUTPUT ${SPIRV_OUT}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_BIN_DIR}
        COMMAND ${GLSLC_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER} -o ${SHADER_BIN_DIR}/${FILE_NAME}.spv
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER}
        VERBATIM
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV_OUT})
endforeach()

add_custom_target(compile_shaders ALL DEPENDS ${SPIRV_BINARY_FILES})
add_dependencies(${EXE_NAME} compile_shaders)
