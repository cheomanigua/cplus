cmake_minimum_required(VERSION 3.24)

project(NavalSim
    VERSION 0.1.0
    DESCRIPTION "Real-time naval warfare simulation"
    LANGUAGES C CXX
)

# --------------------------------------------------
# Language configuration
# --------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    "${CMAKE_BINARY_DIR}/$<CONFIG>"
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
    "${CMAKE_BINARY_DIR}/$<CONFIG>"
)

# --------------------------------------------------
# Project options
# --------------------------------------------------

option(NAVALSIM_WARNINGS_AS_ERRORS
    "Treat compiler warnings as errors"
    OFF
)

option(NAVALSIM_BUILD_CLIENT
    "Build the SDL3 application when SDL3 is available"
    ON
)

# --------------------------------------------------
# External dependencies
# --------------------------------------------------

if(NAVALSIM_BUILD_CLIENT)
    find_package(SDL3 3.2.18 CONFIG QUIET COMPONENTS SDL3)
    if(NOT TARGET SDL3::SDL3)
        message(WARNING
            "SDL3 3.2.18 or newer was not found; the client executable will be skipped. "
            "Core, data, world, simulation, and test targets remain available."
        )
        set(NAVALSIM_BUILD_CLIENT OFF)
    endif()
endif()

# Flecs
set(FLECS_ROOT
    "${PROJECT_SOURCE_DIR}/third-party/flecs"
    CACHE PATH "Path to the Flecs distribution"
)

set(FLECS_HEADER "${FLECS_ROOT}/flecs.h")
set(FLECS_OBJECT "${FLECS_ROOT}/flecs.o")

if(NOT EXISTS "${FLECS_HEADER}")
    message(FATAL_ERROR "Flecs header not found: ${FLECS_HEADER}")
endif()

if(NOT EXISTS "${FLECS_OBJECT}")
    message(FATAL_ERROR "Flecs object file not found: ${FLECS_OBJECT}")
endif()

add_library(flecs_headers INTERFACE)
add_library(flecs::flecs ALIAS flecs_headers)

target_include_directories(flecs_headers INTERFACE
    "${FLECS_ROOT}"
)

# --------------------------------------------------
# doctest
# --------------------------------------------------

include(FetchContent)

FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.4.12
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(doctest)

# --------------------------------------------------
# nlohmann/json
# --------------------------------------------------

set(JSON_BuildTests OFF CACHE INTERNAL "")

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(nlohmann_json)

# --------------------------------------------------
# Common compiler configuration
# --------------------------------------------------

function(navalsim_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE
            /W4
            $<$<BOOL:${NAVALSIM_WARNINGS_AS_ERRORS}>:/WX>
        )
    else()
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            $<$<BOOL:${NAVALSIM_WARNINGS_AS_ERRORS}>:-Werror>
        )
    endif()
endfunction()

# --------------------------------------------------
# Core
# --------------------------------------------------

add_library(navalsim_core INTERFACE)

target_include_directories(navalsim_core INTERFACE
    "${PROJECT_SOURCE_DIR}/src"
)

target_compile_features(navalsim_core INTERFACE
    cxx_std_20
)

# --------------------------------------------------
# Data
# --------------------------------------------------

add_library(navalsim_data STATIC
    src/data/json/json_reader.cpp
	src/data/json/json_object_validator.cpp
	src/data/definitions/component_definition.cpp
	src/data/definitions/component_collection_loader.cpp
	src/data/definitions/definition_loading_configuration_loader.cpp
    src/data/definitions/definition_loading_service.cpp
	src/data/definitions/unit_definition.cpp
	src/data/definitions/unit_collection_loader.cpp
	src/data/definitions/ammunition_definition.cpp
	src/data/definitions/ammunition_collection_loader.cpp
)

target_link_libraries(navalsim_data PUBLIC
    navalsim_core
    nlohmann_json::nlohmann_json
)

navalsim_set_warnings(navalsim_data)

# --------------------------------------------------
# World
# --------------------------------------------------

add_library(navalsim_world STATIC
    src/world/world.cpp
    "${FLECS_OBJECT}"
)

target_link_libraries(navalsim_world PUBLIC
    navalsim_core
    flecs::flecs
)

navalsim_set_warnings(navalsim_world)

# --------------------------------------------------
# Simulation
# --------------------------------------------------

add_library(navalsim_simulation STATIC
    src/simulation/simulation.cpp
    src/simulation/clock/simulation_clock.cpp
    src/simulation/scheduler/simulation_scheduler.cpp
)

target_link_libraries(navalsim_simulation PUBLIC
    navalsim_core
    navalsim_data
    navalsim_world
    navalsim_movement
)

navalsim_set_warnings(navalsim_simulation)

# --------------------------------------------------
# Client
# --------------------------------------------------

if(NAVALSIM_BUILD_CLIENT)
    add_library(navalsim_client STATIC
        src/client/application.cpp
    )

    target_link_libraries(navalsim_client PUBLIC
        navalsim_core
        navalsim_simulation
        SDL3::SDL3
    )

    navalsim_set_warnings(navalsim_client)
endif()

# --------------------------------------------------
# Domains
# --------------------------------------------------

add_library(navalsim_movement STATIC
    src/domains/movement/movement_systems.cpp
    src/domains/movement/movement_command_processor.cpp
)

target_link_libraries(navalsim_movement PUBLIC
    navalsim_core
    navalsim_world
)

navalsim_set_warnings(navalsim_movement)

# --------------------------------------------------
# Executable
# --------------------------------------------------

if(NAVALSIM_BUILD_CLIENT)
    add_executable(navalsim
        src/main.cpp
    )

    target_link_libraries(navalsim PRIVATE
        navalsim_client
    )

    navalsim_set_warnings(navalsim)
endif()

# --------------------------------------------------
# Tests
# --------------------------------------------------

enable_testing()

add_subdirectory(tests)

