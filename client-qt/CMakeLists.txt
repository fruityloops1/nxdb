cmake_minimum_required(VERSION 3.16)

project(client-qt VERSION 0.1 LANGUAGES C CXX ASM)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

include_directories(Include)
include_directories(${PROJECT_SOURCE_DIR}/Library/enet/include)
#include_directories(${PROJECT_SOURCE_DIR}/Library/penet/Include)
include_directories(${PROJECT_SOURCE_DIR}/../penet/Include)
include_directories(${PROJECT_SOURCE_DIR}/Library/LibNerve/Include)
include_directories(${PROJECT_SOURCE_DIR}/Library/buddy_alloc)
include_directories(${PROJECT_SOURCE_DIR}/Library/zstd/lib)

add_subdirectory(${PROJECT_SOURCE_DIR}/Library/enet)
add_subdirectory(${PROJECT_SOURCE_DIR}/../penet ${CMAKE_CURRENT_BINARY_DIR}/penet)
add_subdirectory(${PROJECT_SOURCE_DIR}/Library/LibNerve)

file(GLOB_RECURSE INCLUDES_H ${PROJECT_SOURCE_DIR}/Include/*.h)
file(GLOB_RECURSE SOURCES_H ${PROJECT_SOURCE_DIR}/Source/*.h)
file(GLOB_RECURSE SOURCES_CXX ${PROJECT_SOURCE_DIR}/Source/*.cpp)
file(GLOB_RECURSE SOURCES_UI ${PROJECT_SOURCE_DIR}/Source/*.ui)
file(GLOB SOURCES_ZSTD ${PROJECT_SOURCE_DIR}/Library/zstd/lib/common/*.c)
file(GLOB SOURCES_ZSTD_COMPRESS ${PROJECT_SOURCE_DIR}/Library/zstd/lib/compress/*.c)
file(GLOB SOURCES_ZSTD_DECOMPRESS ${PROJECT_SOURCE_DIR}/Library/zstd/lib/decompress/*.c)
file(GLOB SOURCES_ZSTD_DECOMPRESS_ASM ${PROJECT_SOURCE_DIR}/Library/zstd/lib/decompress/*.S)

set(PROJECT_SOURCES
        ${SOURCES_CXX}
        ${SOURCES_H}
        ${INCLUDES_H}
        ${SOURCES_ZSTD}
        ${SOURCES_ZSTD_COMPRESS}
        ${SOURCES_ZSTD_DECOMPRESS}
        ${SOURCES_ZSTD_DECOMPRESS_ASM}
        ${SOURCES_UI}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(client-qt
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        Source/memory.h Source/memory.cpp Source/memory.ui
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET client-qt APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(client-qt SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(client-qt
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(client-qt PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(client-qt PUBLIC enet penet Nerve)
target_link_libraries(client-qt PRIVATE Threads::Threads)
if(WIN32)
  target_link_libraries(peepa-server PRIVATE wsock32 ws2_32 winmm)
endif()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.client-qt)
endif()
set_target_properties(client-qt PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS client-qt
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(client-qt)
endif()
