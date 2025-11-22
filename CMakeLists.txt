cmake_minimum_required(VERSION 3.15)
project(LauzHack_SBB_Ticketing_System VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# place built executable in build/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# build libraries in subdirectoriess
add_subdirectory(Core)
add_subdirectory(Gui)

# build the executable from main.cpp
add_executable(main main.cpp)

# link the libraries we build in the subdirectories
target_link_libraries(main PRIVATE core gui)

# enable sensible warnings
target_compile_options(main PRIVATE -Wall -Wextra -Wpedantic)

# make headers from Core/Gui available to the main target (pub contains public headers)
target_include_directories(main PRIVATE
	${CMAKE_SOURCE_DIR}/Core/pub
	${CMAKE_SOURCE_DIR}/Gui/pub
)
