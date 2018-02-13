enable_testing()
if(ENABLE_DEMOS)
    add_subdirectory(demos)
endif()
#TODO: Fix autopilot
#add_subdirectory(autopilot)
add_test(
    NAME
        autopilot
    COMMAND
        ${CMAKE_CURRENT_SOURCE_DIR}/autopilot/test.sh
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/autopilot
)
