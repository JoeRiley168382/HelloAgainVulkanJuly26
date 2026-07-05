cmake_minimum_required(VERSION 3.21)
project(VulkanPractice0726 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Lib)

# Vulkan SDK (VULKAN_SDK env var) — headers + glslc only, no link against vulkan-1.lib
# since volk loads all entry points dynamically at runtime.
find_package(Vulkan REQUIRED COMPONENTS glslc)

# --- SDL3, fetched from source ---
include(FetchContent)
set(SDL_STATIC ON  CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-3.2.16
)
FetchContent_MakeAvailable(SDL3)

# --- Shader compilation ---
file(GLOB_RECURSE SHADER_SOURCES
    ${CMAKE_SOURCE_DIR}/Source_shaders/*.vert
    ${CMAKE_SOURCE_DIR}/Source_shaders/*.frag
)
set(SHADER_OUTPUTS "")
foreach(shader ${SHADER_SOURCES})
    get_filename_component(shader_name ${shader} NAME)
    set(shader_out ${CMAKE_SOURCE_DIR}/Bin_shaders/${shader_name}.spv)
    add_custom_command(
        OUTPUT  ${shader_out}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/Bin_shaders
        COMMAND Vulkan::glslc ${shader} -o ${shader_out}
        DEPENDS ${shader}
        VERBATIM
    )
    list(APPEND SHADER_OUTPUTS ${shader_out})
endforeach()
add_custom_target(Shaders DEPENDS ${SHADER_OUTPUTS})

# --- VulkanApp static library (volk.c lives here since it owns Vulkan loading) ---
file(GLOB_RECURSE VULKANAPP_SOURCES
    ${CMAKE_SOURCE_DIR}/Source_support/VulkanApp/*.cpp
    ${CMAKE_SOURCE_DIR}/Source_support/VulkanApp/*.h
)

add_library(VulkanApp STATIC
    ${VULKANAPP_SOURCES}
    ${CMAKE_SOURCE_DIR}/External/volk.c
)

target_include_directories(VulkanApp PUBLIC
    ${CMAKE_SOURCE_DIR}/Source_support/VulkanApp
    ${CMAKE_SOURCE_DIR}/Source_support
    ${CMAKE_SOURCE_DIR}/External
    ${Vulkan_INCLUDE_DIRS}
    $<TARGET_PROPERTY:SDL3::SDL3,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(VulkanApp PUBLIC SDL3::SDL3)

target_compile_options(VulkanApp PRIVATE
    $<$<CONFIG:Debug>:/Od /Zi /W4>
    $<$<CONFIG:Release>:/O2 /W4>
)

# --- One library per subdirectory of Source_support (except VulkanApp) ---
file(GLOB SUPPORT_DIRS LIST_DIRECTORIES true ${CMAKE_SOURCE_DIR}/Source_support/*)
set(SUPPORT_LIBS "")
foreach(dir ${SUPPORT_DIRS})
    if(IS_DIRECTORY ${dir})
        get_filename_component(lib_name ${dir} NAME)
        if(lib_name STREQUAL "VulkanApp")
            continue()
        endif()
        file(GLOB_RECURSE lib_sources ${dir}/*.cpp ${dir}/*.h)
        if(lib_sources)
            add_library(${lib_name} STATIC ${lib_sources})
            target_include_directories(${lib_name} PUBLIC
                ${dir}
                ${CMAKE_SOURCE_DIR}/Source_support
            )
            target_link_libraries(${lib_name} PUBLIC VulkanApp)
            if(lib_name STREQUAL "VulkanRenderer")
                target_compile_definitions(${lib_name} PRIVATE
                    SHADER_DIR="${CMAKE_SOURCE_DIR}/Bin_shaders/"
                )
            endif()
            target_compile_options(${lib_name} PRIVATE
                $<$<CONFIG:Debug>:/Od /Zi /W4>
                $<$<CONFIG:Release>:/O2 /W4>
            )
            list(APPEND SUPPORT_LIBS ${lib_name})
        endif()
    endif()
endforeach()

# --- One executable per .cpp in Source_apps/ ---
file(GLOB APP_MAINS ${CMAKE_SOURCE_DIR}/Source_apps/*.cpp)

foreach(app_main ${APP_MAINS})
    get_filename_component(app_name ${app_main} NAME_WE)

    add_executable(${app_name} ${app_main})

    target_link_libraries(${app_name} PRIVATE ${SUPPORT_LIBS})
    add_dependencies(${app_name} Shaders)

    target_compile_options(${app_name} PRIVATE
        $<$<CONFIG:Debug>:/Od /Zi /W4>
        $<$<CONFIG:Release>:/O2 /W4>
    )
endforeach()
