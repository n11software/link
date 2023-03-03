cmake_minimum_required(VERSION 3.20.2)
project(Link)

# Library
file(GLOB_RECURSE SOURCES Source/*.cpp)

include_directories(Include)
include_directories(/usr/local/include)
add_library(link SHARED ${SOURCES})

target_compile_features(link PUBLIC cxx_std_17)

target_include_directories(link PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_link_libraries(link PUBLIC ssl crypto z)


install(TARGETS link
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Include/
        DESTINATION /usr/local/include
        FILES_MATCHING PATTERN "*.hpp")

# Test
file(GLOB_RECURSE TEST_SOURCES Test/*.cpp)

add_executable(linktest ${TEST_SOURCES})
target_link_libraries(linktest PRIVATE link)

enable_testing()

add_test(NAME test COMMAND $<TARGET_FILE:linktest>)
