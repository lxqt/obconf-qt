project(obconf-qt)
# newer cmake is required for CMAKE_AUTOMOC Qt support
cmake_minimum_required(VERSION 2.8.6)

find_program(SED_PROGRAM sed)

find_package(Qt4 REQUIRED QtCore QtGui) # Qt
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
  # FIXME: is this the proper place to install an icon?
  DESTINATION share/pixmaps
)

# install a desktop entry file
configure_file(data/obconf-qt.desktop.in obconf-qt.desktop @ONLY)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/obconf-qt.desktop
  DESTINATION share/applications
)

# update translations
add_custom_target(update_translations ALL DEPENDS
  obconf-qt_translations
)

# building tarball with CPack -------------------------------------------------
# To create a source distribution, type:
# make package_source
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set (CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README")
set (CPACK_PACKAGE_VENDOR "")
set (CPACK_PACKAGE_VERSION_MAJOR "0")
set (CPACK_PACKAGE_VERSION_MINOR "1")
set (CPACK_PACKAGE_VERSION_PATCH "0")
set (CPACK_GENERATOR TBZ2)
set (CPACK_SOURCE_GENERATOR TBZ2)
set (CPACK_SOURCE_IGNORE_FILES /build/;.gitignore;.*~;.git;.kdev4;temp)
# set (CPACK_SOURCE_PACKAGE_FILE_NAME pcmanfm-qt.0.1.0.tar.gz)
include (CPack)
