cmake_minimum_required(VERSION 3.10)

project(drone_swarm C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -g -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=600)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(Threads REQUIRED)

set(COMMON_SOURCES
    src/utils.c
)

# --- Main ---
add_executable(drone_swarm
    src/main.c
    src/drone_manager.c
    src/commander_manager.c
    src/operator_manager.c
    ${COMMON_SOURCES}
)
target_include_directories(drone_swarm PRIVATE include)
target_link_libraries(drone_swarm PRIVATE Threads::Threads m)

# --- Drone worker ---
add_executable(drone_worker
    src/drone_worker.c
    ${COMMON_SOURCES}
)
target_include_directories(drone_worker PRIVATE include)
target_link_libraries(drone_worker PRIVATE Threads::Threads m)