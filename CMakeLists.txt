cmake_minimum_required(VERSION 3.20)
include(FetchContent)
project(moltenUI CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(LIB_NAME ${PROJECT_NAME})
set(EXE_NAME moltenUI_demo)
set(ASSET_BIN_DIR "$<TARGET_FILE_DIR:${LIB_NAME}>/assets")
find_package(Vulkan REQUIRED)
find_package(glfw3 3.4 REQUIRED)
find_package(Freetype REQUIRED)

FetchContent_Declare(fetch_vk_bootstrap GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap GIT_TAG main)
FetchContent_Declare(fetch_vma GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator GIT_TAG master)
FetchContent_MakeAvailable(fetch_vk_bootstrap fetch_vma)

include_directories(include)

add_library(${LIB_NAME} 
    src/lib.cpp
    src/init.cpp
    src/render.cpp
    src/shader.cpp
    src/ui.cpp
    src/components/input.cpp
    src/font.cpp
)

target_include_directories(${LIB_NAME} PRIVATE
    ${FREETYPE_INCLUDE_DIRS}
)
target_compile_options(${LIB_NAME} PRIVATE -Wno-nullability-completeness)
target_link_libraries(${LIB_NAME} PUBLIC Vulkan::Vulkan glfw vk-bootstrap::vk-bootstrap VulkanMemoryAllocator ${FREETYPE_LIBRARIES})

add_executable(${EXE_NAME} demo/main.cpp)
target_link_libraries(${EXE_NAME} PRIVATE ${LIB_NAME})

find_program(NAGA_EXECUTABLE naga)
find_program(GLSLC_EXECUTABLE glslc)

if(NOT NAGA_EXECUTABLE AND NOT GLSLC_EXECUTABLE)
    message(FATAL_ERROR "Neither naga nor glslc found. Install one of them.")
endif()

set(SHADER_FILES
    shaders/rect.wgsl
    shaders/text.wgsl
)

set(SHADER_BIN_DIR "$<TARGET_FILE_DIR:${EXE_NAME}>/shaders")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/shaders_temp")

foreach(SHADER ${SHADER_FILES})
    get_filename_component(FILE_NAME ${SHADER} NAME_WE)
    set(SPIRV_OUT "${CMAKE_CURRENT_BINARY_DIR}/shaders_temp/${FILE_NAME}.spv")
    
    if(NAGA_EXECUTABLE)
        add_custom_command(
            OUTPUT ${SPIRV_OUT}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_BIN_DIR}
            COMMAND ${NAGA_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER} ${SHADER_BIN_DIR}/${FILE_NAME}.spv
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER}
            VERBATIM
        )
    else()
        message(ERROR "install naga to compile shaders")
    endif()
    
    list(APPEND SPIRV_BINARY_FILES ${SPIRV_OUT})
endforeach()

add_custom_target(shaders ALL DEPENDS ${SPIRV_BINARY_FILES})
add_dependencies(${EXE_NAME} shaders)

add_custom_target(compile_shaders ALL DEPENDS ${SPIRV_BINARY_FILES})
add_dependencies(${EXE_NAME} compile_shaders)

add_custom_command(
    TARGET ${EXE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${ASSET_BIN_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
            ${CMAKE_CURRENT_SOURCE_DIR}/assets ${ASSET_BIN_DIR}
    COMMENT "Copying assets to build directory..."
)