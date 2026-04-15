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

# =====================================================================
# C++ source patches: QRegExp removal and other Qt6 API changes
# =====================================================================

# --- QDoubleSpinBoxG.h ---
# QString::sprintf → static QString::asprintf
# QLocale::groupSeparator() now returns QString instead of QChar
set(_file "${SOURCE_DIR}/src/QDoubleSpinBoxG.h")
file(READ "${_file}" _content)
string(REPLACE "str.sprintf(" "str = QString::asprintf(" _content "${_content}")
string(REPLACE "locale().groupSeparator().isSpace()" "locale().groupSeparator().at(0).isSpace()" _content "${_content}")
string(REPLACE "locale().groupSeparator().isPrint()" "locale().groupSeparator().at(0).isPrint()" _content "${_content}")
file(WRITE "${_file}" "${_content}")

# --- qtpropertymanager.h ---
# QRegExp → QRegularExpression
set(_file "${SOURCE_DIR}/src/qtpropertymanager.h")
file(READ "${_file}" _content)
string(REPLACE "QRegExp" "QRegularExpression" _content "${_content}")
file(WRITE "${_file}" "${_content}")

# --- qtpropertymanager.cpp ---
set(_file "${SOURCE_DIR}/src/qtpropertymanager.cpp")
file(READ "${_file}" _content)
# Blanket type rename (also converts QRegExpValidator → QRegularExpressionValidator)
string(REPLACE "QRegExp" "QRegularExpression" _content "${_content}")
# Fix Data constructor — note the double space before Qt:: in the original source
string(REPLACE
  "regExp(QString(QLatin1Char('*')),  Qt::CaseSensitive, QRegularExpression::Wildcard)"
  "regExp(QRegularExpression::wildcardToRegularExpression(QString(QLatin1Char('*'))))"
  _content "${_content}")
# exactMatch() does not exist in QRegularExpression
string(REPLACE "exactMatch(val)" "match(val).hasMatch()" _content "${_content}")
# Add include
string(REPLACE
  "#include \"qtpropertymanager.h\""
  "#include \"qtpropertymanager.h\"\n#include <QRegularExpression>"
  _content "${_content}")
file(WRITE "${_file}" "${_content}")

# --- qteditorfactory.h ---
set(_file "${SOURCE_DIR}/src/qteditorfactory.h")
file(READ "${_file}" _content)
string(REPLACE "QRegExp" "QRegularExpression" _content "${_content}")
file(WRITE "${_file}" "${_content}")

# --- qteditorfactory.cpp ---
set(_file "${SOURCE_DIR}/src/qteditorfactory.cpp")
file(READ "${_file}" _content)
# QRegExp/QRegExpValidator → QRegularExpression/QRegularExpressionValidator
string(REPLACE "QRegExp" "QRegularExpression" _content "${_content}")
# QLayout::setMargin(int) removed in Qt6
string(REPLACE "setMargin(0)" "setContentsMargins(0, 0, 0, 0)" _content "${_content}")
# QStyleOption::init → initFrom
string(REPLACE "opt.init(this)" "opt.initFrom(this)" _content "${_content}")
# QColorDialog::getRgba removed in Qt6 — use getColor instead
string(REPLACE [=[    bool ok = false;
    QRgb oldRgba = m_color.rgba();
    QRgb newRgba = QColorDialog::getRgba(oldRgba, &ok, this);
    if (ok && newRgba != oldRgba) {
        setValue(QColor::fromRgba(newRgba));]=]
[=[    QColor newColor = QColorDialog::getColor(m_color, this, QString(), QColorDialog::ShowAlphaChannel);
    if (newColor.isValid() && newColor != m_color) {
        setValue(newColor);]=]
  _content "${_content}")
# Add includes
string(REPLACE
  "#include \"qteditorfactory.h\""
  "#include \"qteditorfactory.h\"\n#include <QRegularExpression>\n#include <QRegularExpressionValidator>"
  _content "${_content}")
file(WRITE "${_file}" "${_content}")

# --- qtvariantproperty.h ---
set(_file "${SOURCE_DIR}/src/qtvariantproperty.h")
file(READ "${_file}" _content)
string(REPLACE "QRegExp" "QRegularExpression" _content "${_content}")
file(WRITE "${_file}" "${_content}")

# --- qtvariantproperty.cpp ---
set(_file "${SOURCE_DIR}/src/qtvariantproperty.cpp")
file(READ "${_file}" _content)
# QRegExp → QRegularExpression (type in signatures and qvariant_cast)
string(REPLACE "QRegExp" "QRegularExpression" _content "${_content}")
# QVariant::RegExp removed in Qt6 (not caught by blanket replace since "RegExp" != "QRegExp")
string(REPLACE "QVariant::RegExp" "QVariant::RegularExpression" _content "${_content}")
# Add include
string(REPLACE
  "#include \"qtvariantproperty.h\""
  "#include \"qtvariantproperty.h\"\n#include <QRegularExpression>"
  _content "${_content}")
file(WRITE "${_file}" "${_content}")

# --- qttreepropertybrowser.cpp ---
set(_file "${SOURCE_DIR}/src/qttreepropertybrowser.cpp")
file(READ "${_file}" _content)
# QLayout::setMargin removed in Qt6
string(REPLACE "setMargin(0)" "setContentsMargins(0, 0, 0, 0)" _content "${_content}")
# QTreeWidget::setItemExpanded removed — use QTreeWidgetItem::setExpanded
string(REPLACE "m_treeWidget->setItemExpanded(newItem, true)" "newItem->setExpanded(true)" _content "${_content}")
file(WRITE "${_file}" "${_content}")

# --- qtpropertybrowserutils.cpp ---
set(_file "${SOURCE_DIR}/src/qtpropertybrowserutils.cpp")
file(READ "${_file}" _content)
# QLayout::setMargin removed in Qt6
string(REPLACE "setMargin(0)" "setContentsMargins(0, 0, 0, 0)" _content "${_content}")
# QStyleOption::init → initFrom
string(REPLACE "opt.init(this)" "opt.initFrom(this)" _content "${_content}")
file(WRITE "${_file}" "${_content}")

message(STATUS "QtPropertyBrowser patched for Qt6")
