# Donation Tracker - A Qt-based application for managing donations
# Copyright (C) 2025 Russ Wright russ.wright@gmail.com
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
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

QT += core gui widgets sql
TARGET = donation_tracker
TEMPLATE = app
SOURCES += main.cpp
HEADERS += donation_tracker.h
LIBS += -lsqlite3
QMAKE_CXXFLAGS += -fPIC
