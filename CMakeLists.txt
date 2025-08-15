cmake_minimum_required(VERSION 3.20)

project(ucanutils-x86-test LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

################################################################################
# Default build type
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(WARNING "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
  "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

################################################################################
# Headers and Sources
set(project_incdirs
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)

file(GLOB_RECURSE project_sources CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/test/*.cpp
)

################################################################################
# External dependencies
include(FetchContent)

FetchContent_Declare(emblib
    GIT_REPOSITORY git@github.com:OlegAushev/emblib.git
    GIT_TAG master
    SOURCE_SUBDIR "MADE-UP-DIRECTORY"
)

FetchContent_MakeAvailable(emblib)

################################################################################
# Executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(test-executable ${CMAKE_PROJECT_NAME})

add_executable(${test-executable}
    ${project_sources}
)

target_compile_definitions(${test-executable} PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
)

target_include_directories(${test-executable} PRIVATE
    ${project_incdirs}
    ${FETCHCONTENT_BASE_DIR}/emblib-src/include
)

################################################################################
# Compiler Options and Features
target_compile_options(${test-executable} PRIVATE
    $<$<CONFIG:Debug>:-Og -ggdb3>
    $<$<CONFIG:Release>:-O3 -g0>
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -Wshadow
    -Wdouble-promotion
    -Wconversion
    -Wsign-conversion
    -Wsign-compare
    -Wold-style-cast
    -Wuseless-cast
    -Wsuggest-override
    -fdiagnostics-color=always
)

target_compile_features(${test-executable} PRIVATE
    c_std_11
    cxx_std_23
)
