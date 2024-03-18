QT 	  += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = USBLineCamera8MSystem
TEMPLATE = lib
CONFIG += plugin
DEFINES += QT_DEPRECATED_WARNINGS

SHAREDIR = $$shell_path($$PWD/../../octproz_share_dev)
PLUGINEXPORTDIR = $$shell_path($$SHAREDIR/plugins)
unix{
	OUTFILE = $$shell_path($$OUT_PWD/lib$$TARGET'.'$${QMAKE_EXTENSION_SHLIB})
}
win32{
	CONFIG(debug, debug|release) {
		OUTFILE = $$shell_path($$OUT_PWD/debug/$$TARGET'.'$${QMAKE_EXTENSION_SHLIB})
	}
	CONFIG(release, debug|release) {
		OUTFILE = $$shell_path($$OUT_PWD/release/$$TARGET'.'$${QMAKE_EXTENSION_SHLIB})
	}
}


INCLUDEPATH += $$SHAREDIR

SOURCES += \
	src/usblinecamera8msystemsettingsdialog.cpp \
	src/usblinecamera8msystem.cpp \
	src/usblinecamera8m.cpp \
	thirdparty/coptonix/pipethread.cpp \
	serialcompanel/comportselectwidget.cpp \
	serialcompanel/octserialcom.cpp \
	serialcompanel/octserialcompanel.cpp

HEADERS += \
	$$SHAREDIR/octproz_devkit.h \
	src/usblinecamera8msystem.h \
	src/usblinecamera8msystemsettingsdialog.h \
	src/usblinecamera8m.h \
	src/usblinecamera8msettings.h \
	thirdparty/coptonix/pipethread.h \
	serialcompanel/comportselectwidget.h \
	serialcompanel/octserialcom.h \
	serialcompanel/octserialcompanel.h

FORMS += \
	src/usblinecamera8msystemsettingsdialog.ui \
	serialcompanel/comportselectwidget.ui

RESOURCES += \
	resources.qrc


CONFIG(debug, debug|release) {
	PLUGINEXPORTDIR = $$shell_path($$SHAREDIR/plugins/debug)
	unix{
		LIBS += $$shell_path($$SHAREDIR/debug/libOCTproZ_DevKit.a)
	}
	win32{
		LIBS += $$shell_path($$SHAREDIR/debug/OCTproZ_DevKit.lib)
	}
}
CONFIG(release, debug|release) {
	PLUGINEXPORTDIR = $$shell_path($$SHAREDIR/plugins/release)
	unix{
		LIBS += $$shell_path($$SHAREDIR/release/libOCTproZ_DevKit.a)
	}
	win32{
		LIBS += $$shell_path($$SHAREDIR/release/OCTproZ_DevKit.lib)
	}
}

##Create PLUGINEXPORTDIR directory if not already existing
exists($$PLUGINEXPORTDIR){
		message("plugindir already existing")
	}else{
		QMAKE_PRE_LINK += $$sprintf($$QMAKE_MKDIR_CMD, $$quote($${PLUGINEXPORTDIR})) $$escape_expand(\\n\\t)
}

##Copy shared lib to "PLUGINEXPORTDIR"
unix{
	QMAKE_POST_LINK += $$QMAKE_COPY $$quote($${OUTFILE}) $$quote($$PLUGINEXPORTDIR) $$escape_expand(\\n\\t)
}
win32{
	QMAKE_POST_LINK += $$QMAKE_COPY $$quote($${OUTFILE}) $$quote($$shell_path($$PLUGINEXPORTDIR/$$TARGET'.'$${QMAKE_EXTENSION_SHLIB})) $$escape_expand(\\n\\t)
}

##Add plugin to clean directive. When running "make clean" plugin will be deleted
unix {
	QMAKE_CLEAN += $$shell_path($$PLUGINEXPORTDIR/lib$$TARGET'.'$${QMAKE_EXTENSION_SHLIB})
}
win32 {
	QMAKE_CLEAN += $$shell_path($$PLUGINEXPORTDIR/$$TARGET'.'$${QMAKE_EXTENSION_SHLIB})
}


##USB Line Camera 8M Library
unix:!macx: LIBS += -L$$PWD/usblc8mlib/ -lusblc8mjtn

INCLUDEPATH += $$PWD/usblc8mlib \
		thirdparty/coptonix
DEPENDPATH += $$PWD/usblc8mlib

