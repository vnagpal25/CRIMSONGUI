# Patch script to convert QtPropertyBrowser from Qt5 to Qt6.
# Called via PATCH_COMMAND with -DSOURCE_DIR=<SOURCE_DIR>

if(NOT DEFINED SOURCE_DIR)
  message(FATAL_ERROR "SOURCE_DIR must be defined")
endif()

# --- Patch top-level CMakeLists.txt ---
set(_file "${SOURCE_DIR}/CMakeLists.txt")
file(READ "${_file}" _content)
string(REPLACE "CMAKE_MINIMUM_REQUIRED(VERSION 2.8.2)" "CMAKE_MINIMUM_REQUIRED(VERSION 3.16)" _content "${_content}")
string(REPLACE "FIND_PACKAGE(Qt5 COMPONENTS Widgets REQUIRED)" "FIND_PACKAGE(Qt6 COMPONENTS Widgets REQUIRED)" _content "${_content}")
file(WRITE "${_file}" "${_content}")

# --- Patch src/CMakeLists.txt ---
set(_file "${SOURCE_DIR}/src/CMakeLists.txt")
file(READ "${_file}" _content)
string(REPLACE "QT5_WRAP_UI" "QT6_WRAP_UI" _content "${_content}")
string(REPLACE "QT5_WRAP_CPP" "QT6_WRAP_CPP" _content "${_content}")
string(REPLACE "QT5_ADD_RESOURCES" "QT6_ADD_RESOURCES" _content "${_content}")
string(REPLACE "qt5_use_modules(\${libname} Widgets Gui Core)" "target_link_libraries(\${libname} Qt6::Widgets Qt6::Gui Qt6::Core)" _content "${_content}")
file(WRITE "${_file}" "${_content}")

message(STATUS "QtPropertyBrowser patched for Qt6")
