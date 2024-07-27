set(PLUGIN_NAME "sys-monitor")
add_compile_definitions(PLUGIN_NAME="${PLUGIN_NAME}")

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Dtk COMPONENTS Core Gui REQUIRED)
find_package(DdeTrayLoader REQUIRED)

# @ref https://cmake.org/cmake/help/v3.12/command/file.html#glob-recurse
file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS
    "*.h" "*.cpp" "*.ui"
)
qt5_add_resources(QRC_FILE ${PLUGIN_NAME}.qrc)

add_library(${PROJECT_NAME} MODULE ${SRCS} ${QRC_FILE})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PLUGIN_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC
    Qt5::Widgets
    Dtk::Core
    Dtk::Gui
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Widgets
    Dtk::Core
    Dtk::Gui
)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/dde-dock/plugins)
