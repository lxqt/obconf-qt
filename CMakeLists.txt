project(obconf-qt)
# newer cmake is required for CMAKE_AUTOMOC Qt support
cmake_minimum_required(VERSION 2.8.12)

find_program(SED_PROGRAM sed)

include(GNUInstallDirs)

set(LXQTBT_MINIMUM_VERSION "0.1.0")

# Support Qt4 for the time being
option(USE_QT4 "Build with Qt4." $ENV{USE_QT4})
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_AUTOMOC ON)

if(USE_QT4)
  find_package(Qt4 REQUIRED QtCore QtGui) # Qt
  include(${QT_USE_FILE})
  message(STATUS "Building with Qt${QTVERSION}")
else()
  cmake_minimum_required(VERSION 2.8.11)
  find_package(Qt5Widgets REQUIRED)
  find_package(Qt5X11Extras REQUIRED)
  find_package(Qt5LinguistTools REQUIRED QUIET)
  message(STATUS "Building with Qt${Qt5Core_VERSION}")
endif()

find_package(lxqt-build-tools ${LXQTBT_MINIMUM_VERSION} REQUIRED)

find_package(PkgConfig)
pkg_check_modules(GLIB REQUIRED
  glib-2.0
)

pkg_check_modules(OPENBOX REQUIRED
  obrender-3.5
  obt-3.5
)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(OBCONFQT_MAJOR_VERSION 0)
set(OBCONFQT_MINOR_VERSION 12)
set(OBCONFQT_PATCH_VERSION 0)
set(OBCONFQT_VERSION ${OBCONFQT_MAJOR_VERSION}.${OBCONFQT_MINOR_VERSION}.${OBCONFQT_PATCH_VERSION})
add_definitions("-DOBCONFQT_VERSION=\"${OBCONFQT_VERSION}\"")

include(LXQtTranslateTs)
include(LXQtTranslateDesktop)
include(LXQtCompilerSettings)

# add src subdirectory
add_subdirectory(src)

# install an icon for the application
install(
  FILES data/obconf-qt.png
  DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/48x48/apps"
)
