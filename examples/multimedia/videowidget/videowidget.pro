TEMPLATE = app

QT += widgets multimedia

HEADERS = \
    videoplayer.h \
    videowidget.h \
    videowidgetsurface.h

SOURCES = \
    main.cpp \
    videoplayer.cpp \
    videowidget.cpp \
    videowidgetsurface.cpp

# install
target.path = $$[QT_INSTALL_EXAMPLES]/qtmultimedia/multimedia/videowidget
sources.files = $$SOURCES $$HEADERS $$FORMS $$RESOURCES *.pro *.png images
sources.path = $$[QT_INSTALL_EXAMPLES]/qtmultimedia/multimedia/videowidget
INSTALLS += target sources

symbian {
    TARGET.UID3 = 0xA000D7C3
    CONFIG += qt_example
}
maemo5: include($$QT_SOURCE_TREE/examples/maemo5pkgrules.pri)

symbian: warning(This example might not fully work on Symbian platform)
