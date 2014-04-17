TEMPLATE = subdirs

config_gpu_vivante {
    SUBDIRS += imx6
}

contains(QT_CONFIG, egl): SUBDIRS += egl
