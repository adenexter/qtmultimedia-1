HEADERS       = audiooutput.h
SOURCES       = audiooutput.cpp \
                main.cpp

QT           += widgets multimedia

# install
target.path = $$[QT_INSTALL_EXAMPLES]/qtmultimedia/multimedia/audiooutput
sources.files = $$SOURCES *.h $$RESOURCES $$FORMS audiooutput.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qtmultimedia/multimedia/audiooutput
INSTALLS += target sources

symbian {
    TARGET.UID3 = 0xA000D7C0
    CONFIG += qt_example
}
maemo5: include($$QT_SOURCE_TREE/examples/maemo5pkgrules.pri)
maemo5: warning(This example might not fully work on Maemo platform)
