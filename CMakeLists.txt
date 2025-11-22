cmake_minimum_required(VERSION 3.15)
project(LauzHack_SBB_Ticketing_System VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable Qt automatic features (moc/uic/rcc) if Qt is available
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Prefer Qt6 but fall back to Qt5 if necessary
find_package(Qt6 COMPONENTS Core Gui Widgets QUIET)
if(Qt6_FOUND)
	message(STATUS "Found Qt6")
	set(QT_CORE Qt6::Core)
	set(QT_GUI Qt6::Gui)
	set(QT_WIDGETS Qt6::Widgets)
else()
	find_package(Qt5 COMPONENTS Core Gui Widgets QUIET)
	if(Qt5_FOUND)
		message(STATUS "Found Qt5")
		set(QT_CORE Qt5::Core)
		set(QT_GUI Qt5::Gui)
		set(QT_WIDGETS Qt5::Widgets)
	endif()
endif()
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

# Create a small INTERFACE target to expose project-wide headers (root folder)
# so any target can simply `#include "definitions.h"` when placed in project root.
add_library(project_includes INTERFACE)
target_include_directories(project_includes INTERFACE ${CMAKE_SOURCE_DIR})

# Link the interface to our libraries and executable so the include path is available
if(TARGET core)
    target_link_libraries(core PUBLIC project_includes)
endif()
if(TARGET gui)
    target_link_libraries(gui PUBLIC project_includes)
endif()
target_link_libraries(main PRIVATE project_includes)

# If Qt was found, link the Qt components to our libraries so all targets
# have access to Qt. We link as PUBLIC so consumers also get include paths.
if(TARGET core OR TARGET gui)
	if(DEFINED QT_CORE)
		if(TARGET core)
			target_link_libraries(core PUBLIC ${QT_CORE} ${QT_GUI} ${QT_WIDGETS})
		endif()
		if(TARGET gui)
			target_link_libraries(gui PUBLIC ${QT_CORE} ${QT_GUI} ${QT_WIDGETS})
		endif()
		# Ensure the final executable also links Qt (in case it uses Qt directly)
		target_link_libraries(main PRIVATE ${QT_CORE} ${QT_GUI} ${QT_WIDGETS})
	endif()
endif()
