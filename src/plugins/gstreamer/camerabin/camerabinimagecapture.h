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


#ifndef CAMERABINIMAGECAPTURECONTROL_H
#define CAMERABINIMAGECAPTURECONTROL_H

#include <qcameraimagecapturecontrol.h>
#include "camerabinsession.h"

#include <qvideosurfaceformat.h>

#include <private/qgstreamerbufferprobe_p.h>

QT_BEGIN_NAMESPACE

class CameraBinImageCapture : public QCameraImageCaptureControl, public QGstreamerBusMessageFilter
{
    Q_OBJECT
    Q_INTERFACES(QGstreamerBusMessageFilter)
public:
    CameraBinImageCapture(CameraBinSession *session);
    virtual ~CameraBinImageCapture();

    QCameraImageCapture::DriveMode driveMode() const { return QCameraImageCapture::SingleImageCapture; }
    void setDriveMode(QCameraImageCapture::DriveMode) {}

    bool isReadyForCapture() const;
    int capture(const QString &fileName);
    void cancelCapture();

    bool processBusMessage(const QGstreamerMessage &message);

private slots:
    void updateState();

private:
#if GST_CHECK_VERSION(1,0,0)
    static GstPadProbeReturn encoderEventProbe(GstPad *, GstPadProbeInfo *info, gpointer user_data);
#else
    static gboolean encoderEventProbe(GstElement *, GstEvent *event, gpointer user_data);
#endif

    class EncoderProbe : public QGstreamerBufferProbe
    {
    public:
        EncoderProbe(CameraBinImageCapture *capture) : capture(capture) {}
        void probeCaps(GstCaps *caps);
        bool probeBuffer(GstBuffer *buffer);

    private:
        CameraBinImageCapture * const capture;
    } m_encoderProbe;

    class MuxerProbe : public QGstreamerBufferProbe
    {
    public:
        MuxerProbe(CameraBinImageCapture *capture) : capture(capture) {}
        void probeCaps(GstCaps *caps);
        bool probeBuffer(GstBuffer *buffer);

    private:
        CameraBinImageCapture * const capture;

    } m_muxerProbe;

    QVideoSurfaceFormat m_bufferFormat;
    QSize m_jpegResolution;
    CameraBinSession *m_session;
    GstElement *m_jpegEncoderElement;
    GstElement *m_metadataMuxerElement;
    int m_bytesPerLine;
    int m_requestId;
    bool m_ready;
};

QT_END_NAMESPACE

#endif // CAMERABINCAPTURECORNTROL_H
