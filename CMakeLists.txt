project(obconf-qt)
# newer cmake is required for CMAKE_AUTOMOC Qt support
cmake_minimum_required(VERSION 2.8.6)

find_program(SED_PROGRAM sed)

# additional cmake files
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(GNUInstallDirs)

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
  message(STATUS "Building with Qt${Qt5Core_VERSION_STRING}")
endif()

#Note: no run-time dependency on liblxqt, just a build dependency for lxqt_translate_ts/desktop
find_package(lxqt REQUIRED)

find_package(PkgConfig)
pkg_check_modules(GLIB REQUIRED
  glib-2.0
)

pkg_check_modules(OPENBOX REQUIRED
  obrender-3.5
  obt-3.5
)

# add src subdirectory
add_subdirectory(src)

# install an icon for the application
install(
  FILES data/obconf-qt.png
  DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/48x48/apps"
)
