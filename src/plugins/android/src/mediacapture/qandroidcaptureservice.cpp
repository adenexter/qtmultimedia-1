/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qandroidcaptureservice.h"

#include "qandroidcameracontrol.h"
#include "qandroidvideodeviceselectorcontrol.h"
#include "qandroidcamerasession.h"
#include "qandroidvideorendercontrol.h"
#include "qandroidcamerazoomcontrol.h"
#include "qandroidcameraexposurecontrol.h"
#include "qandroidcameraflashcontrol.h"
#include "qandroidcamerafocuscontrol.h"
#include "qandroidcameralockscontrol.h"
#include "qandroidcameraimageprocessingcontrol.h"
#include "qandroidimageencodercontrol.h"
#include "qandroidcameraimagecapturecontrol.h"
#include "qandroidcameracapturedestinationcontrol.h"
#include "qandroidcameracapturebufferformatcontrol.h"

QT_BEGIN_NAMESPACE

QAndroidCaptureService::QAndroidCaptureService(QObject *parent)
    : QMediaService(parent)
    , m_videoRendererControl(0)
{
    m_cameraSession = new QAndroidCameraSession;
    m_cameraControl = new QAndroidCameraControl(m_cameraSession);
    m_videoInputControl = new QAndroidVideoDeviceSelectorControl(m_cameraSession);
    m_cameraZoomControl = new QAndroidCameraZoomControl(m_cameraSession);
    m_cameraExposureControl = new QAndroidCameraExposureControl(m_cameraSession);
    m_cameraFlashControl = new QAndroidCameraFlashControl(m_cameraSession);
    m_cameraFocusControl = new QAndroidCameraFocusControl(m_cameraSession);
    m_cameraLocksControl = new QAndroidCameraLocksControl(m_cameraSession);
    m_cameraImageProcessingControl = new QAndroidCameraImageProcessingControl(m_cameraSession);
    m_imageEncoderControl = new QAndroidImageEncoderControl(m_cameraSession);
    m_imageCaptureControl = new QAndroidCameraImageCaptureControl(m_cameraSession);
    m_captureDestinationControl = new QAndroidCameraCaptureDestinationControl(m_cameraSession);
    m_captureBufferFormatControl = new QAndroidCameraCaptureBufferFormatControl;
}

QAndroidCaptureService::~QAndroidCaptureService()
{
    delete m_cameraControl;
    delete m_videoInputControl;
    delete m_cameraSession;
    delete m_videoRendererControl;
    delete m_cameraZoomControl;
    delete m_cameraExposureControl;
    delete m_cameraFlashControl;
    delete m_cameraFocusControl;
    delete m_cameraLocksControl;
    delete m_cameraImageProcessingControl;
    delete m_imageEncoderControl;
    delete m_imageCaptureControl;
    delete m_captureDestinationControl;
    delete m_captureBufferFormatControl;
}

QMediaControl *QAndroidCaptureService::requestControl(const char *name)
{
    if (qstrcmp(name, QCameraControl_iid) == 0)
        return m_cameraControl;

    if (qstrcmp(name, QVideoDeviceSelectorControl_iid) == 0)
        return m_videoInputControl;

    if (qstrcmp(name, QCameraZoomControl_iid) == 0)
        return m_cameraZoomControl;

    if (qstrcmp(name, QCameraExposureControl_iid) == 0)
        return m_cameraExposureControl;

    if (qstrcmp(name, QCameraFlashControl_iid) == 0)
        return m_cameraFlashControl;

    if (qstrcmp(name, QCameraFocusControl_iid) == 0)
        return m_cameraFocusControl;

    if (qstrcmp(name, QCameraLocksControl_iid) == 0)
        return m_cameraLocksControl;

    if (qstrcmp(name, QCameraImageProcessingControl_iid) == 0)
        return m_cameraImageProcessingControl;

    if (qstrcmp(name, QImageEncoderControl_iid) == 0)
        return m_imageEncoderControl;

    if (qstrcmp(name, QCameraImageCaptureControl_iid) == 0)
        return m_imageCaptureControl;

    if (qstrcmp(name, QCameraCaptureDestinationControl_iid) == 0)
        return m_captureDestinationControl;

    if (qstrcmp(name, QCameraCaptureBufferFormatControl_iid) == 0)
        return m_captureBufferFormatControl;

    if (qstrcmp(name, QVideoRendererControl_iid) == 0) {
        if (!m_videoRendererControl) {
            m_videoRendererControl = new QAndroidVideoRendererControl;
            m_cameraSession->setVideoPreview(m_videoRendererControl);
            return m_videoRendererControl;
        }
    }

    return 0;
}

void QAndroidCaptureService::releaseControl(QMediaControl *control)
{
    if (control == m_videoRendererControl) {
        m_cameraSession->setVideoPreview(0);
        delete m_videoRendererControl;
        m_videoRendererControl = 0;
    }
}

QT_END_NAMESPACE
