cmake_minimum_required(VERSION 3.15)

# Create a static library target for Gui using sources from pub/*.cpp
file(GLOB_RECURSE GUI_PUB_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/pub/*.cpp)
file(GLOB_RECURSE GUI_PRIV_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/priv/*.cpp)
file(GLOB_RECURSE GUI_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/pub/*.h ${CMAKE_CURRENT_SOURCE_DIR}/priv/*.h)

if(GUI_PUB_SOURCES OR GUI_PRIV_SOURCES)
    add_library(gui STATIC ${GUI_PUB_SOURCES} ${GUI_PRIV_SOURCES} ${GUI_HEADERS})

    # Public headers (API) live in 'pub', private headers in 'priv'
    target_include_directories(gui
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/pub
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/priv
    )

    # Enable AUTOMOC for Qt meta-object compilation
    set_target_properties(gui PROPERTIES AUTOMOC ON)

    # If Gui depends on Core, link it here (adjust if not needed)
    target_link_libraries(gui PUBLIC core)

    # Inherit C++ standard from parent project
    target_compile_features(gui PUBLIC cxx_std_17)

    # Sensible warnings for library as well
    target_compile_options(gui PRIVATE -Wall -Wextra -Wpedantic)
else()
    message(STATUS "No source files found in ${CMAKE_CURRENT_SOURCE_DIR}/pub for target 'gui'; creating INTERFACE target for headers only.")
    add_library(gui INTERFACE)
    # expose public headers so other targets can include them
    target_include_directories(gui INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/pub)
endif()
