TEMPLATE	= app
CONFIG		+= qt warn_on release
QT		+= core gui network
greaterThan(QT_MAJOR_VERSION, 4) {
QT += widgets printsupport
}

isEmpty(PREFIX) {
PREFIX=/usr/local
}

# Automatic platform detection
macx {
HAVE_MACOSX=YES
}
ios {
HAVE_IOS=YES
}

!isEmpty(HAVE_IOS) {
icons.path	= $${PREFIX}
icons.files	= qcclient_72x72.png qcclient_114x114.png qcclient_120x120.png qcclient_144x144.png qcclient_152x152.png qcclient_57x57.png qcclient_76x76.png
QMAKE_BUNDLE_DATA += icons
QMAKE_INFO_PLIST= qcclient_ios.plist
HAVE_BUNDLE_ICONS=YES
QMAKE_APPLE_DEVICE_ARCHS= armv7 arm64
QMAKE_IOS_DEPLOYMENT_TARGET= 9.2
}

!isEmpty(HAVE_MACOSX) {
icons.path	= $${PREFIX}
icons.files	= qcclient.icns
QMAKE_BUNDLE_DATA += icons
QMAKE_INFO_PLIST= qcclient_osx.plist
HAVE_BUNDLE_ICONS=YES
}

HEADERS		+= qcclient.h
HEADERS		+= qcclient_button.h
HEADERS		+= qcclient_day.h
HEADERS		+= qcclient_edit.h
HEADERS		+= qcclient_month.h
HEADERS		+= qcclient_week.h
HEADERS		+= qcclient_year.h
HEADERS		+= qcclient_database.h

SOURCES		+= qcclient.cpp
SOURCES		+= qcclient_button.cpp
SOURCES		+= qcclient_day.cpp
SOURCES		+= qcclient_edit.cpp
SOURCES		+= qcclient_month.cpp
SOURCES		+= qcclient_week.cpp
SOURCES		+= qcclient_year.cpp
SOURCES		+= qcclient_database.cpp

RESOURCES	+= qcclient.qrc

TARGET		= QCalendarClient

target.path	= $${PREFIX}/bin
INSTALLS	+= target

isEmpty(HAVE_BUNDLE_ICONS) {
  icons.path	= $${PREFIX}/share/pixmaps
  icons.files	= qcclient_48x48.png
  INSTALLS	+= icons
}

desktop.path	= $${PREFIX}/share/applications
desktop.files	= qcclient.desktop
INSTALLS	+= desktop

