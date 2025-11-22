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

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ---- Find RNP (cross-platform) ---------------------------------
# Try pkg-config first (good on Linux / some BSDs)
find_package(PkgConfig)

set(RNP_FOUND FALSE)
if(PkgConfig_FOUND)
    # Try both names; distros may call the module 'rnp' or 'librnp'
    pkg_check_modules(RNP QUIET rnp)
    if(NOT RNP_FOUND)
        pkg_check_modules(RNP QUIET librnp)
    endif()
endif()

if(RNP_FOUND)
    # pkg-config gave us everything we need
    set(RNP_INCLUDE_DIRS ${RNP_INCLUDE_DIRS})
    set(RNP_LIBRARIES    ${RNP_LIBRARIES})
else()
    # Fallback for systems without a .pc file (e.g. Homebrew on macOS)
    # You can override this with: cmake -DRNP_ROOT=/path/to/rnp ..
    set(RNP_HINTS "")

    if(RNP_ROOT)
        list(APPEND RNP_HINTS "${RNP_ROOT}")
    endif()

    # Common prefixes:
    list(APPEND RNP_HINTS
        /opt/homebrew/opt/rnp           # macOS Apple Silicon Homebrew
        /usr/local                      # macOS Intel / generic
        /usr                            # system install
        /home/linuxbrew/.linuxbrew/opt/rnp  # Linux Homebrew default
    )

    find_path(RNP_INCLUDE_DIR
        NAMES rnp/rnp.h
        PATH_SUFFIXES include
        HINTS ${RNP_HINTS}
    )

    find_library(RNP_LIBRARY
        NAMES rnp
        PATH_SUFFIXES lib
        HINTS ${RNP_HINTS}
    )

    if(NOT RNP_INCLUDE_DIR OR NOT RNP_LIBRARY)
        message(FATAL_ERROR "Could not find RNP (rnp/rnp.h and librnp). \
Set RNP_ROOT or install the development package (e.g. 'brew install rnp' on macOS, \
or the appropriate rnp-dev package on Linux).")
    endif()

    # librnp (Homebrew ships it as a static .a) also needs these dependencies linked explicitly.
    find_library(RNP_SEXPP_LIBRARY
        NAMES sexpp libsexpp
        PATH_SUFFIXES lib
        HINTS ${RNP_HINTS}
    )
    find_library(RNP_BOTAN_LIBRARY
        NAMES botan-3 libbotan-3 botan
        PATH_SUFFIXES lib
        HINTS ${RNP_HINTS} /opt/homebrew/opt/botan@3 /opt/homebrew/opt/botan
    )
    find_library(RNP_JSONC_LIBRARY
        NAMES json-c libjson-c
        PATH_SUFFIXES lib
        HINTS ${RNP_HINTS} /opt/homebrew/opt/json-c
    )
    find_package(BZip2 REQUIRED)
    find_package(ZLIB  REQUIRED)

    if(NOT RNP_SEXPP_LIBRARY OR NOT RNP_BOTAN_LIBRARY OR NOT RNP_JSONC_LIBRARY)
        message(FATAL_ERROR "Found librnp at ${RNP_LIBRARY} but missing dependencies (sexpp, botan, json-c). \
Install them or set the appropriate *_ROOT cache variables.")
    endif()

    set(RNP_INCLUDE_DIRS "${RNP_INCLUDE_DIR}")
    set(RNP_LIBRARIES
        "${RNP_LIBRARY}"
        "${RNP_SEXPP_LIBRARY}"
        "${RNP_BOTAN_LIBRARY}"
        "${RNP_JSONC_LIBRARY}"
        ${BZIP2_LIBRARIES}
        ${ZLIB_LIBRARIES}
    )
endif()

# ---- Subdirectories --------------------------------------------
add_subdirectory(Core)
add_subdirectory(Gui)

# ---- Executable ------------------------------------------------
add_executable(main main.cpp)

target_link_libraries(main
    PRIVATE
        core
        gui
        ${RNP_LIBRARIES}
)

target_compile_options(main PRIVATE -Wall -Wextra -Wpedantic)

target_include_directories(main PRIVATE
    ${CMAKE_SOURCE_DIR}/Core/pub
    ${CMAKE_SOURCE_DIR}/Gui/pub
    ${RNP_INCLUDE_DIRS}
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

# If Core/Gui code also includes <rnp/rnp.h>, give them the same deps:
target_link_libraries(core PRIVATE ${RNP_LIBRARIES})
target_link_libraries(gui  PRIVATE ${RNP_LIBRARIES})

target_include_directories(core PRIVATE ${RNP_INCLUDE_DIRS})
target_include_directories(gui  PRIVATE ${RNP_INCLUDE_DIRS})
