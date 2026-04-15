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

# --- Patch QRegExp → QRegularExpression in C++ sources ---
# Qt6 removed QRegExp from QtCore; replace with QRegularExpression.

# qtpropertymanager.h: simple type replacement
set(_file "${SOURCE_DIR}/src/qtpropertymanager.h")
file(READ "${_file}" _content)
string(REPLACE "QRegExp" "QRegularExpression" _content "${_content}")
file(WRITE "${_file}" "${_content}")

# qtpropertymanager.cpp: type replacement + API changes
set(_file "${SOURCE_DIR}/src/qtpropertymanager.cpp")
file(READ "${_file}" _content)
# 1) Blanket type rename (also converts QRegExpValidator → QRegularExpressionValidator)
string(REPLACE "QRegExp" "QRegularExpression" _content "${_content}")
# 2) Fix Data constructor: QRegularExpression::Wildcard doesn't exist
string(REPLACE
  "regExp(QString(QLatin1Char('*')), Qt::CaseSensitive, QRegularExpression::Wildcard)"
  "regExp(QRegularExpression::wildcardToRegularExpression(QString(QLatin1Char('*'))))"
  _content "${_content}")
# 3) exactMatch() doesn't exist in QRegularExpression
string(REPLACE "exactMatch(val)" "match(val).hasMatch()" _content "${_content}")
# 4) Add missing include
string(REPLACE
  "#include \"qtpropertymanager.h\""
  "#include \"qtpropertymanager.h\"\n#include <QRegularExpression>"
  _content "${_content}")
file(WRITE "${_file}" "${_content}")

# qteditorfactory.h: type replacement in Q_PRIVATE_SLOT
set(_file "${SOURCE_DIR}/src/qteditorfactory.h")
file(READ "${_file}" _content)
string(REPLACE "QRegExp" "QRegularExpression" _content "${_content}")
file(WRITE "${_file}" "${_content}")

# qteditorfactory.cpp: type + validator replacement
set(_file "${SOURCE_DIR}/src/qteditorfactory.cpp")
file(READ "${_file}" _content)
string(REPLACE "QRegExp" "QRegularExpression" _content "${_content}")
# Add missing include
string(REPLACE
  "#include \"qteditorfactory.h\""
  "#include \"qteditorfactory.h\"\n#include <QRegularExpression>\n#include <QRegularExpressionValidator>"
  _content "${_content}")
file(WRITE "${_file}" "${_content}")

message(STATUS "QtPropertyBrowser patched for Qt6")
