HEADERS       = audioinput.h
SOURCES       = audioinput.cpp \
                main.cpp

QT           += widgets multimedia

# install
target.path = $$[QT_INSTALL_EXAMPLES]/qtmultimedia/multimedia/audioinput
sources.files = $$SOURCES *.h $$RESOURCES $$FORMS audioinput.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qtmultimedia/multimedia/audioinput
INSTALLS += target sources

symbian {
    TARGET.UID3 = 0xA000D7BF
    TARGET.CAPABILITY += UserEnvironment
    CONFIG += qt_example
}
maemo5: include($$QT_SOURCE_TREE/examples/maemo5pkgrules.pri)

maemo5: warning(This example might not fully work on Maemo platform)
