cmake_minimum_required(VERSION 3.16)

project(link C CXX)

include_directories(Includes)

set(INSTALL_LIBDIR /usr/lib)
set(INSTALL_INCLUDEDIR /usr/include)

file(GLOB_RECURSE CPPSourceCode Source/*.cpp)
file(GLOB_RECURSE Headers Includes/*.h)

set(CMAKE_CXX_FLAGS "-fpic -pthread")
add_link_options(-pthread)
ADD_LIBRARY(link SHARED ${CPPSourceCode})

set_target_properties(link PROPERTIES PUBLIC_HEADER "${Headers}")

set(LIBRARY_VERSION_MAJOR 1)
set(LIBRARY_VERSION_MINOR 0.1)

add_custom_command(TARGET link POST_BUILD
	COMMAND cp liblink.so /usr/lib/
	COMMAND cp -R Includes/* /usr/include/
	USES_TERMINAL
)
