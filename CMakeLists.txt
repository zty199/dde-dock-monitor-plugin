cmake_minimum_required(VERSION 3.12)

project(dde-dock-monitor-plugin)

if (NOT DEFINED VERSION)
    set(VERSION 2.0.0)
endif ()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(AUTOMOC_COMPILER_PREDEFINES ON)

# Enable Qt6
option(OPT_ENABLE_QT6 "Use Qt6" ON)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

include(GNUInstallDirs)

add_subdirectory(src)
# add_subdirectory(translations)
