cmake_minimum_required(VERSION 3.24.2)
project("Bluetooth")
set (CMAKE_CXX_STANDARD 17)

##List of options:

include_directories("${CMAKE_CURRENT_LIST_DIR}/Header Files")
file(GLOB_RECURSE CPPFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/Source Files/*.cpp")
file(GLOB_RECURSE HFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/Header Files/*.h")
add_library(Bluetooth SHARED ${CPPFiles} ${HFiles})

# Optional: Set the output directory for the DLLs
set_target_properties(Bluetooth PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)