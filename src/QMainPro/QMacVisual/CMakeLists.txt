# ===========================================================================
# QMacVisual — Main Application
# ===========================================================================
if(NOT Qt5_FOUND)
    return()
endif()

# --- Find required Qt modules ---
find_package(Qt5 COMPONENTS Core Gui Widgets Network SerialPort Sql Xml QUIET)

# --- Collect source files ---
file(GLOB QMACVISUAL_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp"
)

file(GLOB QMACVISUAL_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
)

file(GLOB QMACVISUAL_UI_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.ui"
)

set(QMACVISUAL_QRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/res.qrc"
)

set(QMACVISUAL_RC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/QMacVisual.rc"
)

# --- Create executable ---
add_executable(QMacVisual WIN32
    ${QMACVISUAL_SOURCES}
    ${QMACVISUAL_HEADERS}
    ${QMACVISUAL_UI_FILES}
    ${QMACVISUAL_QRC_FILES}
    ${QMACVISUAL_RC_FILES}
)

# --- Qt auto-MOC/UIC/RCC ---
set_target_properties(QMacVisual PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

# --- Qt linking ---
target_link_libraries(QMacVisual PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
    Qt5::SerialPort
    Qt5::Sql
    Qt5::Xml
)

# --- Compile definitions ---
target_compile_definitions(QMacVisual PRIVATE
    BUILD_STATIC
    NOMINMAX
    _CRT_SECURE_NO_WARNINGS
    _SCL_SECURE_NO_WARNINGS
    _WINDOWS
)

# --- Include directories ---
target_include_directories(QMacVisual PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/QWorld"
    "${CMAKE_CURRENT_SOURCE_DIR}/CommTools/Camera/MindVision/Include"
    "${RXS_QMACV_SRC_DIR}/czxDependence"
)

# --- OpenCV ---
find_package(OpenCV QUIET)
if(OpenCV_FOUND)
    target_include_directories(QMacVisual PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(QMacVisual PRIVATE ${OpenCV_LIBS})
endif()

# --- OpenMP ---
find_package(OpenMP QUIET)
if(OpenMP_FOUND)
    target_link_libraries(QMacVisual PRIVATE OpenMP::OpenMP_CXX)
endif()

# --- MSVC settings ---
if(MSVC)
    target_compile_options(QMacVisual PRIVATE
        /utf-8
        /MP
        /W2
        /permissive-
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:Debug>:/Od>
    )
endif()

# --- Output directory ---
set_target_properties(QMacVisual PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

message(STATUS "  App: QMacVisual (${Qt5_VERSION})")
