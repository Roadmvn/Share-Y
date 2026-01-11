cmake_minimum_required(VERSION 3.20)
project(shareY VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Qt6 Auto-generation
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Find XCB libraries for X11 capture
find_package(PkgConfig REQUIRED)
pkg_check_modules(XCB REQUIRED 
    xcb 
    xcb-util 
    xcb-keysyms 
    xcb-image 
    xcb-shm
    xcb-xfixes
)

# Source files
set(SOURCES
    src/main.cpp
    src/core/ScreenshotBuffer.cpp
    src/capture/CaptureEngine.cpp
    src/capture/X11Capture.cpp
    src/hotkeys/HotkeyManager.cpp
    src/hotkeys/X11Hotkey.cpp
    src/ui/MainWindow.cpp
    src/ui/ThumbnailWidget.cpp
    src/ui/RegionSelector.cpp
)

set(HEADERS
    src/core/Screenshot.hpp
    src/core/ScreenshotBuffer.hpp
    src/capture/CaptureEngine.hpp
    src/capture/X11Capture.hpp
    src/hotkeys/HotkeyManager.hpp
    src/hotkeys/X11Hotkey.hpp
    src/ui/MainWindow.hpp
    src/ui/ThumbnailWidget.hpp
    src/ui/RegionSelector.hpp
)

# Resources
set(RESOURCES
    resources/shareY.qrc
)

# Executable
add_executable(${PROJECT_NAME} 
    ${SOURCES} 
    ${HEADERS}
    ${RESOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${XCB_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    ${XCB_LIBRARIES}
)

# Compiler warnings
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall -Wextra -Wpedantic
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# Install target
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(FILES resources/shareY.desktop DESTINATION share/applications)
install(FILES resources/icons/shareY.png DESTINATION share/icons/hicolor/256x256/apps)
