TEMPLATE = lib

TARGET = qgsttools_p
QPRO_PWD = $$PWD
QT = core multimedia-private gui-private

!static:DEFINES += QT_MAKEDLL

unix:!maemo*:contains(QT_CONFIG, alsa) {
DEFINES += HAVE_ALSA
LIBS_PRIVATE += \
    -lasound
}

CONFIG += link_pkgconfig

PKGCONFIG_PRIVATE += \
    gstreamer-1.0 \
    gstreamer-base-1.0 \
    gstreamer-audio-1.0 \
    gstreamer-video-1.0 \
    gstreamer-pbutils-1.0 \
#    gstreamer-interfaces-1.0 \

maemo*: PKGCONFIG_PRIVATE +=gstreamer-plugins-bad-1.0

config_resourcepolicy {
    DEFINES += HAVE_RESOURCE_POLICY
    PKGCONFIG_PRIVATE += libresourceqt5
}

# Header files must go inside source directory of a module
# to be installed by syncqt.
INCLUDEPATH += ../multimedia/gsttools_headers/
VPATH += ../multimedia/gsttools_headers/

PRIVATE_HEADERS += \
    qgstbufferpoolinterface_p.h \
    qgstreamerbushelper_p.h \
    qgstreamermessage_p.h \
    qgstutils_p.h \
    qgstvideobuffer_p.h \
    qvideosurfacegstsink_p.h \
    qgstreamerbufferprobe_p.h \
    qgstreamervideorendererinterface_p.h \
    qgstreameraudioinputselector_p.h \
    qgstreamervideorenderer_p.h \
    qgstreamervideoinputdevicecontrol_p.h \
    qgstcodecsinfo_p.h \
    qgstreamervideoprobecontrol_p.h \
    qgstreameraudioprobecontrol_p.h \
    qgstreamervideowindow_p.h \
#    gstvideoconnector_p.h \

SOURCES += \
    qgstbufferpoolinterface.cpp \
    qgstreamerbushelper.cpp \
    qgstreamermessage.cpp \
    qgstutils.cpp \
    qgstvideobuffer.cpp \
    qvideosurfacegstsink.cpp \
    qgstreamerbufferprobe.cpp \
    qgstreamervideorendererinterface.cpp \
    qgstreameraudioinputselector.cpp \
    qgstreamervideorenderer.cpp \
    qgstreamervideoinputdevicecontrol.cpp \
    qgstcodecsinfo.cpp \
    qgstreamervideoprobecontrol.cpp \
    qgstreameraudioprobecontrol.cpp \
    qgstreamervideowindow.cpp \
#    gstvideoconnector.c

config_gstreamer_appsrc {
    PKGCONFIG_PRIVATE += gstreamer-app-1.0
    PRIVATE_HEADERS += qgstappsrc_p.h
    SOURCES += qgstappsrc.cpp

    DEFINES += HAVE_GST_APPSRC

    LIBS_PRIVATE += -lgstapp-1.0
}

HEADERS += $$PRIVATE_HEADERS

DESTDIR = $$QT.multimedia.libs
target.path = $$[QT_INSTALL_LIBS]

INSTALLS += target
