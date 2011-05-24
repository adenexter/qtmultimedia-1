HEADERS       = audiodevices.h
SOURCES       = audiodevices.cpp \
                main.cpp
FORMS        += audiodevicesbase.ui

QT           += widgets multimedia

# install
target.path = $$[QT_INSTALL_EXAMPLES]/qtmultimedia/multimedia/audiodevices
sources.files = $$SOURCES *.h $$RESOURCES $$FORMS audiodevices.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qtmultimedia/multimedia/audiodevices
INSTALLS += target sources

symbian {
    TARGET.UID3 = 0xA000D7BE
    CONFIG += qt_example
}
maemo5: include($$QT_SOURCE_TREE/examples/maemo5pkgrules.pri)

