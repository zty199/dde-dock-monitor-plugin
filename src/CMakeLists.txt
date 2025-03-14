set(PLUGIN_NAME "sys-monitor")
add_compile_definitions(PLUGIN_NAME="${PLUGIN_NAME}")

set(QT_VERSION_MAJOR 5)
set(DTK_VERSION_MAJOR "")
if(OPT_ENABLE_QT6 MATCHES ON)
    set(QT_VERSION_MAJOR 6)
    set(DTK_VERSION_MAJOR 6)
endif()

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS DBus Widgets REQUIRED)
find_package(Dtk${DTK_VERSION_MAJOR} COMPONENTS Core Gui REQUIRED)

find_package(DdeTrayLoader QUIET)

find_package(PkgConfig REQUIRED)
if(OPT_ENABLE_QT6 MATCHES ON)
    pkg_check_modules(GioQt IMPORTED_TARGET gio-qt6)
else()
    pkg_check_modules(GioQt IMPORTED_TARGET gio-qt)
endif()

# @ref https://cmake.org/cmake/help/v3.12/command/file.html#glob-recurse
file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS
    "*.h" "*.hpp" "*.cpp" "*.ui"
)
qt_add_resources(QRC_FILE ${PLUGIN_NAME}.qrc)

add_library(${PROJECT_NAME} MODULE ${SRCS} ${QRC_FILE})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PLUGIN_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Widgets
    Dtk${DTK_VERSION_MAJOR}::Core
    Dtk${DTK_VERSION_MAJOR}::Gui
    PkgConfig::GioQt
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Widgets
    Dtk${DTK_VERSION_MAJOR}::Core
    Dtk${DTK_VERSION_MAJOR}::Gui
    PkgConfig::GioQt
)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/dde-dock/plugins)
install(FILES ${CMAKE_SOURCE_DIR}/resources/gschema/com.deepin.dde.dock.module.sys-monitor.gschema.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/glib-2.0/schemas)
install(FILES ${CMAKE_SOURCE_DIR}/resources/icons/${PLUGIN_NAME}.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/dde-dock/icons/dcc-setting)
