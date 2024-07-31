#-------------------------------------------------
#
# Project created by QtCreator 2020-03-22T14:04:23
#
#-------------------------------------------------

QT       += core gui printsupport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_LFLAGS += -no-pie
QMAKE_CXXFLAGS += -Wno-deprecated-declarations -Werror=return-type


TARGET = WordTsar
TEMPLATE = app


# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
#DEFINES += QT_DEPRECATED_WARNINGS
CONFIG(release, debug|release){
    DEFINES += NDEBUG
}

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++17

equals(DO_TEST, 1){
    error(DO TEST DEFINED)

}

unix:!macx {
    CONFIG(release, debug|release){
        versionTarget.target = ../src/core/include/version.h
        versionTarget.depends = FORCE
        versionTarget.commands = pwd ; echo update version ... ; ../rev.sh ; touch ../src/core/include/version.h
        PRE_TARGETDEPS += ../src/core/include/version.h
        QMAKE_EXTRA_TARGETS += versionTarget
    }
}

INCLUDEPATH += src/third-party

SOURCES += \
    src/core/codepage/cp437.cpp \
    src/core/document/document.cpp \
    src/core/document/math.cpp \
    src/files/docxfile.cpp \
    src/files/rtf/read/rtfcontrolsymbol.cpp \
    src/files/rtf/read/rtfcontrolword.cpp \
    src/files/rtf/read/rtfelement.cpp \
    src/files/rtf/read/rtfgroup.cpp \
    src/files/rtf/read/rtfparser.cpp \
    src/files/rtf/read/rtfstate.cpp \
    src/files/rtf/read/rtftext.cpp \
    src/gui/misc/overlaywidget.cpp \
    src/main.cpp \
    src/test/test.cpp \
    src/third-party/pugixml/pugixml.cpp \
    src/third-party/utf8proc-2.5.0/utf8proc.c \
    src/third-party/utf8proc-2.5.0/utf8proc_data.c \
    src/third-party/zip/zip.c \
    src/wordtsar.cpp \
    src/files/file.cpp \
    src/files/rtffile.cpp \
    src/files/textfile.cpp \
    src/files/wordstarfile.cpp \
    src/files/rtf/write/rtfwriter.cpp \
    src/gui/editor/editorctrl.cpp \
    src/gui/layout/layout.cpp \
    src/gui/print/printout.cpp \
    src/gui/ruler/rulerctrl.cpp \
    src/gui/spellcheck/centerword.cpp \
    src/gui/spellcheck/clinuxspellcheck.cpp \
    src/gui/spellcheck/cspellcheck.cpp \
    src/gui/spellcheck/cspellcheckword.cpp \
    src/input/wordtsarinput.cpp \
    src/gui/dialogs/qcolorpickerbutton.cpp

HEADERS += \
    src/core/codepage/cp437.h \
    src/core/document/doctstructs.h \
    src/core/document/document.h \
    src/core/document/math.h \
    src/core/include/version.h \
    src/files/docx/constants.hpp \
    src/files/docx/duckxiterator.hpp \
    src/files/docxfile.h \
    src/files/rtf/read/rtfcontrolsymbol.h \
    src/files/rtf/read/rtfcontrolword.h \
    src/files/rtf/read/rtfelement.h \
    src/files/rtf/read/rtfgroup.h \
    src/files/rtf/read/rtfparser.h \
    src/files/rtf/read/rtfstate.h \
    src/files/rtf/read/rtftext.h \
    src/gui/misc/overlaywidget.h \
    src/test/test.h \
    src/third-party/pugixml/pugiconfig.hpp \
    src/third-party/pugixml/pugixml.hpp \
    src/third-party/zip/miniz.h \
    src/third-party/zip/zip.h \
    src/wordtsar.h \
    src/files/file.h \
    src/files/rtffile.h \
    src/files/textfile.h \
    src/files/wordstarfile.h \
    src/files/rtf/structs.h \
    src/files/rtf/write/rtfwriter.h \
    src/gui/editor/editorctrl.h \
    src/gui/layout/layout.h \
    src/gui/print/printout.h \
    src/gui/ruler/rulerctrl.h \
    src/gui/spellcheck/centerword.h \
    src/gui/spellcheck/clinuxspellcheck.h \
    src/gui/spellcheck/cspellcheck.h \
    src/gui/spellcheck/cspellcheckword.h \
    src/core/include/config.h \
    src/input/wordtsarinput.h \
    src/gui/dialogs/qcolorpickerbutton.h

FORMS += \
    src/gui/dialogs/preferences.ui \
    src/gui/dialogs/pagelayout.ui \
    src/gui/dialogs/find.ui \
    src/gui/dialogs/findreplace.ui

RESOURCES += \
    src/resources.qrc

DISTFILES += \
    src/files/docx/readme.txt






