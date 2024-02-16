cmake_minimum_required(VERSION 3.5)

project(strategiesbotwidget VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Network)
find_package(RapidJSON)

set(TS_FILES strategiesbotwidget_en_US.ts)

add_subdirectory(mytwsapi)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(strategiesbotwidget
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ewrapperqthreaded.h ewrapperqthreaded.cpp twsclientqthreaded.h twsclientqthreaded.cpp
        ReqIdTypes.h
        logwindow.h logwindow.cpp logwindow.ui
        earningsvolatilitystrat.h earningsvolatilitystrat.cpp
        rapidJsonHelpers.h
        accountmanager.h accountmanager.cpp
        ordersmanager.h ordersmanager.cpp
        mainwindowdrawing.cpp
        strategies.h
        earningsvolatilitystratmonitor.h earningsvolatilitystratmonitor.cpp
        optionschainagent.h optionschainagent.cpp
        logformatter.h
        agentcontroller.h
        databrokeragent.h databrokeragent.cpp
        marketdatasingleton.h
        marketdatasingleton.cpp
        trendfollowstrat.h trendfollowstrat.cpp
        MultiCandle.h
        IndicatorTypes.h
        ta-lib-wrapper.h ta-lib-wrapper.cpp
        timefuncs.h
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET strategiesbotwidget APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(strategiesbotwidget SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(strategiesbotwidget
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(strategiesbotwidget PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(strategiesbotwidget PRIVATE Qt${QT_VERSION_MAJOR}::Network)
target_link_libraries(strategiesbotwidget PUBLIC mytwsapi)
target_link_libraries(strategiesbotwidget PUBLIC libta_lib.so)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.strategiesbotwidget)
endif()
set_target_properties(strategiesbotwidget PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS strategiesbotwidget
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(strategiesbotwidget)
endif()
