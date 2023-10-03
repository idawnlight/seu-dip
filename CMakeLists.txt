cmake_minimum_required(VERSION 3.26)
project(seu_dip)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Quick
        REQUIRED)

find_package(OpenCV REQUIRED)

file(GLOB_RECURSE QML_SOURCES
        RELATIVE ${PROJECT_SOURCE_DIR}
        CONFIGURE_DEPENDS
        qml/*.qml)

file(GLOB_RECURSE QML_RESOURCES
        RELATIVE ${PROJECT_SOURCE_DIR}
        CONFIGURE_DEPENDS
        res/*)

file(GLOB_RECURSE SOURCES
        CONFIGURE_DEPENDS
        src/*.cpp
        src/*.hpp
)

include_directories(src/qml)

qt_add_executable(seu_dip ${SOURCES})

qt_add_qml_module(seu_dip
        URI dip
        VERSION 1.0
        QML_FILES ${QML_SOURCES}
        RESOURCES ${QML_RESOURCES}
        RESOURCE_PREFIX /
)

target_link_libraries(seu_dip PRIVATE Qt::Widgets Qt::Quick)

target_link_libraries(seu_dip PRIVATE ${OpenCV_LIBS})