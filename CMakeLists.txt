# QFunnyWidgets - C++ / Qt 6 and Qt 5 port.
#
#   cmake -S . -B build                      # picks Qt 6 if found, else Qt 5
#   cmake -S . -B build -DQFW_QT_MAJOR=5     # force Qt 5
#   cmake --build build
#
# Targets:
#   QFunnyWidgets                    static library with all widgets (position independent)
#   funnywidgets-mainwindow          the gallery of all 35 widgets
#   funnywidgets-<package>           one demo per widget package (e.g. funnywidgets-jellyswitch)
#   funnywidgets-makelogos           regenerates <package>/logo.png
#   funnywidgets-designer            launches Qt Designer with the plugin
#   funnywidgetsplugin               Qt Designer plugin (needs the Designer / UiPlugin development files)
#   funnywidgets-smoke               headless smoke test (also registered with CTest)
cmake_minimum_required(VERSION 3.16)
project(QFunnyWidgets VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
include(GNUInstallDirs)

if(WIN32)
    # <windows.h> (pulled in by some Qt headers) must not define min()/max() macros.
    add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

set(QFW_QT_MAJOR "" CACHE STRING "Qt major version to use (5 or 6); empty = Qt 6 if available, else Qt 5")
option(QFW_BUILD_DEMOS "Build one demo executable per widget package" ON)
option(QFW_BUILD_GALLERY "Build the funnywidgets-mainwindow gallery" ON)
option(QFW_BUILD_TOOLS "Build funnywidgets-makelogos and funnywidgets-designer" ON)
option(QFW_BUILD_TESTS "Build the headless smoke test" ON)
option(QFW_BUILD_DESIGNER_PLUGIN "Build the Qt Designer plugin (when the Designer module is available)" ON)

if(QFW_QT_MAJOR STREQUAL "")
    find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
    set(QFW_QT_MAJOR ${QT_VERSION_MAJOR})
endif()
find_package(Qt${QFW_QT_MAJOR} REQUIRED COMPONENTS Widgets)
set(QT Qt${QFW_QT_MAJOR})
message(STATUS "QFunnyWidgets: building against Qt ${${QT}_VERSION}")

# Packages in the order of QFunnyWidgets/__init__.py.
set(QFW_PACKAGES
    googlyeyes runawaybutton jellyswitch hamsterloader moodslider ratingstars holdbutton panicbell
    rubberduck passwordjudge cookieconsent typingindicator emptystate volumeknob pigeontoast dropzone
    batterybuddy presenceavatar ducklingpager deadline searchdetective robotcheck chameleonpicker
    errorgremlin snailstepper pancakestepper heartlike copyblob rockettop catsection dachshundtabs
    puffercounter helpowl antbreadcrumb autosave)

set(QFW_DIR ${CMAKE_CURRENT_SOURCE_DIR}/QFunnyWidgets)
set(QFW_LIB_SOURCES ${QFW_DIR}/QFunnyWidgets.h)
set(QFW_ALL_QRC)
set(QFW_ALL_DEMO_SOURCES)
foreach(pkg IN LISTS QFW_PACKAGES)
    list(APPEND QFW_LIB_SOURCES ${QFW_DIR}/${pkg}/${pkg}.h ${QFW_DIR}/${pkg}/${pkg}.cpp)
    list(APPEND QFW_ALL_QRC ${QFW_DIR}/${pkg}/${pkg}.qrc)
    list(APPEND QFW_ALL_DEMO_SOURCES ${QFW_DIR}/${pkg}/mainwindow.h ${QFW_DIR}/${pkg}/mainwindow.cpp)
endforeach()

# ---------- Library ----------
add_library(QFunnyWidgets STATIC ${QFW_LIB_SOURCES})
add_library(QFunnyWidgets::QFunnyWidgets ALIAS QFunnyWidgets)
target_include_directories(QFunnyWidgets PUBLIC
    $<BUILD_INTERFACE:${QFW_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(QFunnyWidgets PUBLIC ${QT}::Widgets)
target_include_directories(QFunnyWidgets PUBLIC $<INSTALL_INTERFACE:include/QFunnyWidgets>)
set_target_properties(QFunnyWidgets PROPERTIES POSITION_INDEPENDENT_CODE ON)

# ---------- Demos (one per package) ----------
if(QFW_BUILD_DEMOS)
    foreach(pkg IN LISTS QFW_PACKAGES)
        add_executable(funnywidgets-${pkg} WIN32 MACOSX_BUNDLE
            ${QFW_DIR}/${pkg}/main.cpp
            ${QFW_DIR}/${pkg}/mainwindow.h
            ${QFW_DIR}/${pkg}/mainwindow.cpp
            ${QFW_DIR}/${pkg}/${pkg}.qrc)
        target_link_libraries(funnywidgets-${pkg} PRIVATE QFunnyWidgets)
    endforeach()
endif()

# ---------- Gallery ----------
if(QFW_BUILD_GALLERY)
    add_executable(funnywidgets-mainwindow WIN32 MACOSX_BUNDLE
        ${QFW_DIR}/main.cpp ${QFW_DIR}/mainwindow.h ${QFW_DIR}/mainwindow.cpp ${QFW_ALL_QRC})
    target_link_libraries(funnywidgets-mainwindow PRIVATE QFunnyWidgets)
    set_target_properties(funnywidgets-mainwindow PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME "QFunnyWidgets"
        MACOSX_BUNDLE_GUI_IDENTIFIER "io.github.usama01tn.qfunnywidgets.gallery"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}")
endif()

# ---------- Tools ----------
if(QFW_BUILD_TOOLS)
    add_executable(funnywidgets-makelogos ${QFW_DIR}/makelogos.cpp)
    target_link_libraries(funnywidgets-makelogos PRIVATE QFunnyWidgets)
    target_compile_definitions(funnywidgets-makelogos PRIVATE
        QFW_SOURCE_DIR="${QFW_DIR}")

    add_executable(funnywidgets-designer ${QFW_DIR}/designer.cpp)
    target_link_libraries(funnywidgets-designer PRIVATE ${QT}::Core)
    target_compile_definitions(funnywidgets-designer PRIVATE
        QFW_PLUGIN_BUILD_DIR="${CMAKE_CURRENT_BINARY_DIR}/designer"
        QFW_PLUGIN_INSTALL_RELDIR="${CMAKE_INSTALL_LIBDIR}/qt${QFW_QT_MAJOR}/plugins")
endif()

# ---------- Qt Designer plugin ----------
if(QFW_BUILD_DESIGNER_PLUGIN)
    find_package(${QT} QUIET COMPONENTS UiPlugin)
    if(TARGET ${QT}::UiPlugin)
        add_library(funnywidgetsplugin MODULE
            ${CMAKE_CURRENT_SOURCE_DIR}/plugins/funnywidgetsinfo.h
            ${CMAKE_CURRENT_SOURCE_DIR}/plugins/funnywidgetsinfo.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/plugins/funnywidgetsplugin.h
            ${CMAKE_CURRENT_SOURCE_DIR}/plugins/funnywidgetsplugin.cpp
            ${QFW_ALL_QRC})
        target_link_libraries(funnywidgetsplugin PRIVATE QFunnyWidgets ${QT}::UiPlugin)
        target_include_directories(funnywidgetsplugin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/plugins)
        # Designer looks for plugins in <dir>/designer; funnywidgets-designer points QT_PLUGIN_PATH at <dir>.
        # The generator expression stops multi-config generators (Visual Studio, Xcode) from appending
        # a Release/Debug subdirectory, which would hide the plugin from Designer.
        set_target_properties(funnywidgetsplugin PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY $<1:${CMAKE_CURRENT_BINARY_DIR}/designer>)
    else()
        message(STATUS "QFunnyWidgets: ${QT} UiPlugin module not found, skipping the Designer plugin")
    endif()
endif()

# ---------- Smoke test ----------
if(QFW_BUILD_TESTS)
    enable_testing()
    add_executable(funnywidgets-smoke
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/smoke.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/plugins/funnywidgetsinfo.h
        ${CMAKE_CURRENT_SOURCE_DIR}/plugins/funnywidgetsinfo.cpp
        ${QFW_ALL_DEMO_SOURCES} ${QFW_ALL_QRC})
    target_link_libraries(funnywidgets-smoke PRIVATE QFunnyWidgets)
    target_include_directories(funnywidgets-smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/plugins)
    target_compile_definitions(funnywidgets-smoke PRIVATE QFW_SOURCE_DIR="${QFW_DIR}")
    add_test(NAME funnywidgets-smoke COMMAND funnywidgets-smoke)
    set_tests_properties(funnywidgets-smoke PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif()

# ---------- Install ----------
install(TARGETS QFunnyWidgets EXPORT QFunnyWidgetsTargets ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${QFW_DIR}/QFunnyWidgets.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/QFunnyWidgets)
foreach(pkg IN LISTS QFW_PACKAGES)
    install(FILES ${QFW_DIR}/${pkg}/${pkg}.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/QFunnyWidgets/${pkg})
endforeach()
install(EXPORT QFunnyWidgetsTargets NAMESPACE QFunnyWidgets:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QFunnyWidgets)
# find_package(QFunnyWidgets) support: pull in the same Qt major version, then the exported targets.
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/QFunnyWidgetsConfig.cmake
"include(CMakeFindDependencyMacro)
find_dependency(Qt${QFW_QT_MAJOR} COMPONENTS Widgets)
include(\"\${CMAKE_CURRENT_LIST_DIR}/QFunnyWidgetsTargets.cmake\")
")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/QFunnyWidgetsConfig.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QFunnyWidgets)
if(QFW_BUILD_GALLERY)
    install(TARGETS funnywidgets-mainwindow RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} BUNDLE DESTINATION .)
endif()
if(QFW_BUILD_TOOLS)
    install(TARGETS funnywidgets-designer RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
if(TARGET funnywidgetsplugin)
    install(TARGETS funnywidgetsplugin LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/qt${QFW_QT_MAJOR}/plugins/designer)
endif()
