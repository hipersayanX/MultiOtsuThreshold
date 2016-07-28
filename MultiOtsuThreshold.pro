# OtsuThreshold, Implementation of Multi Otsu Threshold in Qt/C++.
# Copyright (C) 2015  Gonzalo Exequiel Pedone
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# Email   : hipersayan DOT x AT gmail DOT com
# Web-Site: http://github.com/hipersayanX/MultiOtsuThreshold
#
# This program is based in the original implementation
# of Multi Otsu Threshold made by Yasunari Tosa for ImageJ:
#
#     http://imagej.net/Multi_Otsu_Threshold

QT += core gui

TARGET = MultiOtsuThreshold
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

RESOURCES += \
    multiotsuthreshold.qrc
