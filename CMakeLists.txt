cmake_minimum_required(VERSION 3.20.2)
project(Link)

# Library
file(GLOB_RECURSE SOURCES Source/*.cpp)

include_directories(Include)
add_library(link SHARED ${SOURCES})

set_target_properties(link PROPERTIES CXX_STANDARD 23)

target_include_directories(link PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_link_libraries(link PUBLIC ssl)

# Test
file(GLOB_RECURSE TEST_SOURCES Test/*.cpp)

add_executable(linktest ${TEST_SOURCES})
target_link_libraries(linktest PRIVATE link)

enable_testing()

add_test(NAME test COMMAND $<TARGET_FILE:linktest>)