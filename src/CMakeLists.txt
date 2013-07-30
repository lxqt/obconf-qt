# set visibility to hidden to hide symbols, unlesss they're exporeted manually in the code
set(CMAKE_CXX_FLAGS "-DQT_NO_KEYWORDS -fno-exceptions")

include_directories(
  ${QT_INCLUDES}
  ${GLIB_INCLUDE_DIRS}
  ${OPENBOX_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}

)

set(CMAKE_AUTOMOC TRUE)

set(obconf-qt_SRCS
  obconf-qt.cpp
  maindialog.cpp
  tree.cpp
  archive.cpp
  theme.cpp
  fontbutton.cpp
)

set(obconf-qt_UIS
  obconf.ui
)

qt4_wrap_ui(obconf-qt_UI_H
  ${obconf-qt_UIS}
)

add_executable(obconf-qt
  ${obconf-qt_SRCS}
  ${obconf-qt_UI_H}
)

add_definitions(
-DPACKAGE_DATA_DIR="${CMAKE_INSTALL_PREFIX}/share/obconf-qt"
)

target_link_libraries(obconf-qt
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${GLIB_LIBRARIES}
  ${OPENBOX_LIBRARIES}
)

install(TARGETS obconf-qt RUNTIME DESTINATION bin)

# add translation for obconf-qt
# See http://www.cmake.org/Wiki/CMake:How_To_Build_Qt4_Software
file(GLOB TS_FILES translations/*.ts)
qt4_create_translation(QM_FILES
  ${obconf-qt_SRCS}
  ${obconf-qt_UI_H}
  ${TS_FILES})
qt4_add_translation(QM_FILES ${TS_FILES})
add_custom_target (obconf-qt_translations DEPENDS ${QM_FILES})
install(FILES ${QM_FILES} DESTINATION share/obconf-qt/translations)

# prevent the generated files from being deleted during make clean
set_directory_properties(PROPERTIES
  CLEAN_NO_CUSTOM true
)
