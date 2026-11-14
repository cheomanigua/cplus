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
# Command queue tests
# --------------------------------------------------

add_executable(navalsim_command_queue_tests
    simulation/commands/command_queue_tests.cpp
)

target_link_libraries(navalsim_command_queue_tests
    PRIVATE
        navalsim_core
        doctest::doctest
)

navalsim_set_warnings(navalsim_command_queue_tests)
doctest_discover_tests(navalsim_command_queue_tests)

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


# --------------------------------------------------
# Command dispatcher tests
# --------------------------------------------------

add_executable(navalsim_command_dispatcher_tests
    simulation/commands/command_dispatcher_tests.cpp
)

target_link_libraries(navalsim_command_dispatcher_tests
    PRIVATE
        navalsim_core
        doctest::doctest
)

navalsim_set_warnings(navalsim_command_dispatcher_tests)
doctest_discover_tests(navalsim_command_dispatcher_tests)


# --------------------------------------------------
# JSON reader tests
# --------------------------------------------------

add_executable(navalsim_json_reader_tests
    data/json_reader_tests.cpp
)

target_link_libraries(navalsim_json_reader_tests PRIVATE
    navalsim_data
    doctest::doctest
)

navalsim_set_warnings(navalsim_json_reader_tests)

doctest_discover_tests(navalsim_json_reader_tests)

# --------------------------------------------------
# JSON object validator tests
# --------------------------------------------------

add_executable(navalsim_json_object_validator_tests
    data/json_object_validator_tests.cpp
)

target_link_libraries(navalsim_json_object_validator_tests PRIVATE
    navalsim_data
    doctest::doctest
)

navalsim_set_warnings(navalsim_json_object_validator_tests)

doctest_discover_tests(navalsim_json_object_validator_tests)


# --------------------------------------------------
# Generic unit definition tests
# --------------------------------------------------

add_executable(navalsim_unit_definition_tests
    data/unit_definition_tests.cpp
)

target_link_libraries(navalsim_unit_definition_tests PRIVATE
    navalsim_data
    doctest::doctest
)

navalsim_set_warnings(navalsim_unit_definition_tests)
doctest_discover_tests(navalsim_unit_definition_tests)

# --------------------------------------------------
# Generic component definition tests
# --------------------------------------------------

add_executable(navalsim_component_definition_tests
    data/component_definition_tests.cpp
)

target_link_libraries(navalsim_component_definition_tests PRIVATE
    navalsim_data
    doctest::doctest
)

navalsim_set_warnings(navalsim_component_definition_tests)
doctest_discover_tests(navalsim_component_definition_tests)


# --------------------------------------------------
# Component collection loader tests
# --------------------------------------------------

add_executable(navalsim_component_collection_loader_tests
    data/component_collection_loader_tests.cpp
)

target_link_libraries(navalsim_component_collection_loader_tests PRIVATE
    navalsim_data
    doctest::doctest
)

navalsim_set_warnings(navalsim_component_collection_loader_tests)
doctest_discover_tests(navalsim_component_collection_loader_tests)


# --------------------------------------------------
# Ammunition definition tests
# --------------------------------------------------

add_executable(navalsim_ammunition_definition_tests
    data/ammunition_definition_tests.cpp
)

target_link_libraries(navalsim_ammunition_definition_tests PRIVATE
    navalsim_data
    doctest::doctest
)

navalsim_set_warnings(navalsim_ammunition_definition_tests)
doctest_discover_tests(navalsim_ammunition_definition_tests)

# --------------------------------------------------
# Ammunition collection loader tests
# --------------------------------------------------


add_executable(navalsim_ammunition_collection_loader_tests
    data/ammunition_collection_loader_tests.cpp
)

target_link_libraries(navalsim_ammunition_collection_loader_tests PRIVATE
    navalsim_data
    doctest::doctest
)

navalsim_set_warnings(navalsim_ammunition_collection_loader_tests)
doctest_discover_tests(navalsim_ammunition_collection_loader_tests)


# --------------------------------------------------
# Definition database tests
# --------------------------------------------------

add_executable(navalsim_definition_database_tests
	data/definition_database_tests.cpp
)

target_link_libraries(navalsim_definition_database_tests PRIVATE
	navalsim_data
	doctest::doctest
)

navalsim_set_warnings(navalsim_definition_database_tests)

doctest_discover_tests(navalsim_definition_database_tests)


# --------------------------------------------------
# Unit collection loader tests
# --------------------------------------------------

add_executable(navalsim_unit_collection_loader_tests
    data/unit_collection_loader_tests.cpp
)

target_link_libraries(navalsim_unit_collection_loader_tests PRIVATE
    navalsim_data
    doctest::doctest
)

navalsim_set_warnings(navalsim_unit_collection_loader_tests)
doctest_discover_tests(navalsim_unit_collection_loader_tests)

# --------------------------------------------------
# Definition batch loader tests
# --------------------------------------------------

add_executable(navalsim_definition_batch_loader_tests
    data/definition_batch_loader_tests.cpp
)

target_link_libraries(navalsim_definition_batch_loader_tests
    PRIVATE
        navalsim_data
        doctest::doctest
)

navalsim_set_warnings(navalsim_definition_batch_loader_tests)
doctest_discover_tests(navalsim_definition_batch_loader_tests)

# --------------------------------------------------
# Definition loader registry tests
# --------------------------------------------------

add_executable(navalsim_definition_loader_registry_tests
    data/definition_loader_registry_tests.cpp
)

target_link_libraries(navalsim_definition_loader_registry_tests
    PRIVATE
        navalsim_data
        doctest::doctest
)

navalsim_set_warnings(navalsim_definition_loader_registry_tests)
doctest_discover_tests(navalsim_definition_loader_registry_tests)

# --------------------------------------------------
# Definition loading configuration tests
# --------------------------------------------------

add_executable(navalsim_definition_loading_configuration_tests
    data/definition_loading_configuration_tests.cpp
)

target_link_libraries(navalsim_definition_loading_configuration_tests
    PRIVATE
        navalsim_data
        doctest::doctest
)

navalsim_set_warnings(navalsim_definition_loading_configuration_tests)
doctest_discover_tests(navalsim_definition_loading_configuration_tests)

# --------------------------------------------------
# Definition loading integration tests
# --------------------------------------------------

add_executable(navalsim_definition_loading_integration_tests
    data/definition_loading_integration_tests.cpp
)
target_link_libraries(navalsim_definition_loading_integration_tests
    PRIVATE navalsim_data doctest::doctest
)
navalsim_set_warnings(navalsim_definition_loading_integration_tests)
doctest_discover_tests(navalsim_definition_loading_integration_tests)
