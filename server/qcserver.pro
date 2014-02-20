TEMPLATE	= app
CONFIG		+= qt warn_on release
QT		+= core network

HEADERS		+= qcserver.h
SOURCES		+= qcserver.cpp

TARGET		= QCalendarServer

target.path	= $${PREFIX}/bin
INSTALLS	+= target
