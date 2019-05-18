CONFIG += c++11
QT += gui
QT += core gui
QT += widgets

HEADERS += EnvironnementGui.h
HEADERS += gAgentGui.h
HEADERS += Environnement.h
HEADERS += VisualAgent.h

SOURCES += main.cpp
SOURCES += Environnement.cpp
SOURCES += EnvironnementGui.cpp
SOURCES += VisualAgent.cpp

TARGET = main