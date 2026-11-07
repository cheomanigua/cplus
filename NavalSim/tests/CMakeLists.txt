# --------------------------------------------------
# doctest integration
# --------------------------------------------------

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)


# --------------------------------------------------
# Clock tests
# --------------------------------------------------

add_executable(navalsim_clock_tests
    simulation/clock/simulation_clock_tests.cpp
)

target_link_libraries(navalsim_clock_tests PRIVATE
    navalsim_simulation
    doctest::doctest
)

doctest_discover_tests(navalsim_clock_tests)

navalsim_set_warnings(navalsim_clock_tests)


# --------------------------------------------------
# Scheduler tests
# --------------------------------------------------

add_executable(navalsim_scheduler_tests
    simulation/scheduler/simulation_scheduler_tests.cpp
)

target_link_libraries(navalsim_scheduler_tests PRIVATE
    navalsim_simulation
    doctest::doctest
)

doctest_discover_tests(navalsim_scheduler_tests)

navalsim_set_warnings(navalsim_scheduler_tests)


# --------------------------------------------------
# Simulation integration tests
# --------------------------------------------------

add_executable(navalsim_simulation_tests
    simulation/simulation_tests.cpp
)

target_link_libraries(navalsim_simulation_tests PRIVATE
    navalsim_simulation
    doctest::doctest
)

doctest_discover_tests(navalsim_simulation_tests)

navalsim_set_warnings(navalsim_simulation_tests)


# --------------------------------------------------
# Geographic types tests
# --------------------------------------------------

add_executable(navalsim_geographic_types_tests
    core/geography/geographic_types_tests.cpp
)

target_link_libraries(navalsim_geographic_types_tests PRIVATE
    navalsim_core
    doctest::doctest
)

doctest_discover_tests(navalsim_geographic_types_tests)

navalsim_set_warnings(navalsim_geographic_types_tests)


# --------------------------------------------------
# Geographic calculations tests
# --------------------------------------------------

add_executable(navalsim_geographic_calculations_tests
    core/geography/geographic_calculations_tests.cpp
)

target_link_libraries(navalsim_geographic_calculations_tests PRIVATE
    navalsim_core
    doctest::doctest
)

doctest_discover_tests(navalsim_geographic_calculations_tests)

navalsim_set_warnings(navalsim_geographic_calculations_tests)


# --------------------------------------------------
# Movement components tests
# --------------------------------------------------

add_executable(
    navalsim_movement_components_tests
    domains/movement/movement_components_tests.cpp
)

target_link_libraries(
    navalsim_movement_components_tests
    PRIVATE
        navalsim_movement
        doctest::doctest
)

navalsim_set_warnings(navalsim_movement_components_tests)

doctest_discover_tests(navalsim_movement_components_tests)


# --------------------------------------------------
# Movement systems tests
# --------------------------------------------------

add_executable(navalsim_movement_systems_tests
    domains/movement/movement_systems_tests.cpp
)

target_link_libraries(navalsim_movement_systems_tests PRIVATE
    navalsim_core
    navalsim_world
    navalsim_movement
    doctest::doctest
)

doctest_discover_tests(navalsim_movement_systems_tests)

navalsim_set_warnings(navalsim_movement_systems_tests)


# --------------------------------------------------
# Movement command processor tests
# --------------------------------------------------

add_executable(navalsim_movement_command_processor_tests
    domains/movement/movement_command_processor_tests.cpp
)

target_link_libraries(navalsim_movement_command_processor_tests
    PRIVATE
        navalsim_core
        navalsim_world
        navalsim_movement
        doctest::doctest
)

navalsim_set_warnings(navalsim_movement_command_processor_tests)
doctest_discover_tests(navalsim_movement_command_processor_tests)
