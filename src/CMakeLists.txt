set(PLUGIN_NAME "sys-monitor")
add_compile_definitions(PLUGIN_NAME="${PLUGIN_NAME}")

find_package(Qt5 COMPONENTS DBus Widgets REQUIRED)
find_package(Dtk COMPONENTS Core Gui REQUIRED)

find_package(DdeTrayLoader QUIET)
if (NOT ${DdeTrayLoader_FOUND})
    find_package(DdeDock REQUIRED)
endif ()

find_package(PkgConfig REQUIRED)
pkg_check_modules(GSettingsQt IMPORTED_TARGET gsettings-qt)

# @ref https://cmake.org/cmake/help/v3.12/command/file.html#glob-recurse
file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS
    "*.h" "*.hpp" "*.cpp" "*.ui"
)
qt5_add_resources(QRC_FILE ${PLUGIN_NAME}.qrc)

add_library(${PROJECT_NAME} MODULE ${SRCS} ${QRC_FILE})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PLUGIN_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC
    Qt5::DBus
    Qt5::Widgets
    Dtk::Core
    Dtk::Gui
    PkgConfig::GSettingsQt
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::DBus
    Qt5::Widgets
    Dtk::Core
    Dtk::Gui
    PkgConfig::GSettingsQt
)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/dde-dock/plugins)
install(FILES ${CMAKE_SOURCE_DIR}/resources/gschema/com.deepin.dde.dock.module.sys-monitor.gschema.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/glib-2.0/schemas)
install(FILES ${CMAKE_SOURCE_DIR}/resources/icons/${PLUGIN_NAME}.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/dde-dock/icons/dcc-setting)
