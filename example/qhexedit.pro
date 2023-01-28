QT       += core gui
LIBS += -lusb-1.0
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

HEADERS = \
    mainwindow.h \
    optionsdialog.h \
    ../src/qhexedit.h \
    ../src/chunks.h \
    ../src/commands.h \
    searchdialog.h \
    eepromsize.h \
    programmer.h \
    counter.h


SOURCES = \
    main.cpp \
    mainwindow.cpp \
    optionsdialog.cpp \
    ../src/qhexedit.cpp \
    ../src/chunks.cpp \
    ../src/commands.cpp \
    searchdialog.cpp \
    eepromsize.cpp \
    programmer.cpp

RESOURCES = \
    qhexedit.qrc

FORMS += \
    optionsdialog.ui \
    searchdialog.ui \
    eepromsize.ui \
    programmer.ui

OTHER_FILES += \
    ../build-example.bat \
    ../build-python-bindings.bat \
    ../build-example.sh \
    ../build-python-bindings.sh \
    ../deploy.nsi \
    ../doc/release.txt \
    ../doc/howtorelease.txt \
    ../appveyor.yml \
    ../readme.md \
    ../setup.py \
    ../src/qhexedit.sip

TRANSLATIONS += \
    translations/qhexedit_cs.ts \
    translations/qhexedit_de.ts \
    translations/qhexedit_ru.ts

DEFINES += QHEXEDIT_EXPORTS
