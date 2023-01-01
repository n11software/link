cmake_minimum_required(VERSION 3.16)

project(link VERSION 1.0.1 LANGUAGES C CXX)

include_directories(Includes)

set(INSTALL_LIBDIR /usr/lib)
set(INSTALL_INCLUDEDIR /usr/include)

file(GLOB_RECURSE CPPSourceCode Source/*.cpp)
file(GLOB_RECURSE Headers Includes/*.h)

set(CMAKE_CXX_FLAGS "-fpic -pthread")
add_link_options(-pthread)
ADD_LIBRARY(link SHARED ${CPPSourceCode} ${Headers})

set_target_properties(link PROPERTIES PUBLIC_HEADER "${Headers}")

set_target_properties(link PROPERTIES
		VERSION ${PROJECT_VERSION}
		SOVERSION ${PROJECT_VERSION_MAJOR})

# Add install target
install(TARGETS link
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})