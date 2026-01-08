cmake_minimum_required(VERSION 3.10)

project(ProjectDrone VERSION 1.0)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/lib")

# Library

add_library(common_lib STATIC
  src/utils.c

  include/utils.h
)

target_include_directories(common_lib PUBLIC
  ${PROJECT_SOURCE_DIR}/include
)

target_compile_options(common_lib PRIVATE
  -Wall -Wextra -pedantic
  $<$<CONFIG:Debug>:-g>
)

# Drone

add_executable(drone_worker
  src/drone_worker.c
)

target_link_libraries(drone_worker PRIVATE
  common_lib
  rt
  pthread
)

target_compile_options(drone_worker PRIVATE
  -Wall -Wextra -pedantic
  $<$<CONFIG:Debug>:-g>
)

# Main

add_executable(${PROJECT_NAME}
  src/main.c
  src/drone_manager.c

  include/drone_manager.h
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  common_lib
  rt
  pthread
)

target_compile_options(${PROJECT_NAME} PRIVATE
  -Wall
  -Wextra
  -pedantic
  $<$<CONFIG:Debug>:-g>
)

# set(SOURCE_FILES
#   src/main.c
#   src/utils.c
#   src/drone_manager.c
#
#   include/utils.h
#   include/drone_manager.h
# )
#
# add_executable(${PROJECT_NAME} ${SOURCE_FILES})
#
# target_compile_options(${PROJECT_NAME} PRIVATE
#   -Wall
#   -Wextra
#   -pedantic
#   $<$<CONFIG:Debug>:-g>
# )
#
# target_include_directories(${PROJECT_NAME} PRIVATE
#   ${PROJECT_SOURCE_DIR}/include
# )
