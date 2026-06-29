# ===========================================================================
# rxs-qmacvisual — 2D 视觉检测平台
# CMake build system (replaces QMacVisual.sln / 55 .vcxproj files)
# ===========================================================================
cmake_minimum_required(VERSION 3.16)

project(rxs_qmacvisual LANGUAGES CXX)

# ---------------------------------------------------------------------------
# Global settings
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Only x64 supported (original projects were x64-only)
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "rxs-qmacvisual requires x64. Please use a 64-bit generator.")
endif()

# ---------------------------------------------------------------------------
# Path constants
# ---------------------------------------------------------------------------
set(RXS_QMACV_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(RXS_QMACV_PLUGIN_OUTPUT_DIR "${CMAKE_BINARY_DIR}/Plugins")

# ---------------------------------------------------------------------------
# Qt5 — find all possible modules at top level
# ---------------------------------------------------------------------------
find_package(Qt5 COMPONENTS Core Gui Widgets Network SerialPort Sql Xml Script QUIET)

if(NOT Qt5_FOUND)
    message(WARNING
        "Qt5 not found. Please set CMAKE_PREFIX_PATH to your Qt installation.\n"
        "Example: cmake -DCMAKE_PREFIX_PATH=\"C:/Qt/5.15.2/msvc2019_64\" ..\n"
        "Plugins requiring Qt will be skipped."
    )
else()
    message(STATUS "Qt5 found: ${Qt5_DIR}")
endif()

# ---------------------------------------------------------------------------
# Include the shared plugin macro
# ---------------------------------------------------------------------------
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/rxs_qmacvisual_plugin.cmake")

# ---------------------------------------------------------------------------
# Main application
# ---------------------------------------------------------------------------
if(Qt5_FOUND)
    add_subdirectory(QMainPro/QMacVisual)
endif()

# ---------------------------------------------------------------------------
# Plugin categories (11 categories, 54 plugins)
# ---------------------------------------------------------------------------
# Each category directory has a CMakeLists.txt that adds its plugins.
add_subdirectory(QPlugins/CalibrationTools)
add_subdirectory(QPlugins/CommunicateTools)
add_subdirectory(QPlugins/DetectIdentify)
add_subdirectory(QPlugins/ExtLibraryCases)
add_subdirectory(QPlugins/GeometryMeasure)
add_subdirectory(QPlugins/GeometryTools)
add_subdirectory(QPlugins/ImageProcess)
add_subdirectory(QPlugins/LogicalTools)
add_subdirectory(QPlugins/ParaTools)
add_subdirectory(QPlugins/SystemTools)
add_subdirectory(QPlugins/ThreeDInspection)

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------
message(STATUS "")
message(STATUS "=== rxs-qmacvisual CMake Configuration ===")
message(STATUS "  C++ Standard:  ${CMAKE_CXX_STANDARD}")
message(STATUS "  Qt5:           ${Qt5_VERSION}")
message(STATUS "  Plugin output: ${RXS_QMACV_PLUGIN_OUTPUT_DIR}")
message(STATUS "  Build type:    ${CMAKE_BUILD_TYPE}")
message(STATUS "=============================================")
