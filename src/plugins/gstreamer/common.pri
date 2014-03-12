
QT += multimedia-private network
CONFIG += no_private_qt_headers_warning

LIBS += -lqgsttools_p

CONFIG += link_pkgconfig

PKGCONFIG += \
    gstreamer-1.0 \
    gstreamer-base-1.0 \
    gstreamer-audio-1.0 \
    gstreamer-video-1.0 \
    gstreamer-pbutils-1.0

config_resourcepolicy {
    DEFINES += HAVE_RESOURCE_POLICY
    PKGCONFIG += libresourceqt5
}

config_gstreamer_appsrc {
    PKGCONFIG += gstreamer-app-1.0
    DEFINES += HAVE_GST_APPSRC
    LIBS += -lgstapp-1.0
}

