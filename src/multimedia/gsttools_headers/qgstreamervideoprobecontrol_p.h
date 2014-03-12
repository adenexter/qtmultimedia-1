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

#ifndef QGSTREAMERVIDEOPROBECONTROL_H
#define QGSTREAMERVIDEOPROBECONTROL_H

#include <gst/gst.h>
#include <qmediavideoprobecontrol.h>
#include <QtCore/qmutex.h>
#include <qvideoframe.h>
#include <qvideosurfaceformat.h>

#include <private/qgstreamerbufferprobe_p.h>

QT_BEGIN_NAMESPACE

class QGstreamerVideoProbeControl : public QMediaVideoProbeControl, public QGstreamerBufferProbe
{
    Q_OBJECT
public:
    explicit QGstreamerVideoProbeControl(QObject *parent);
    virtual ~QGstreamerVideoProbeControl();

    void reference() { ++m_referenceCount; }
    bool release() { return --m_referenceCount == 0; }

    void probeCaps(GstCaps *caps);
    bool probeBuffer(GstBuffer *buffer);

    void startFlushing();
    void stopFlushing();

private slots:
    void frameProbed();

private:
    QVideoSurfaceFormat m_format;
    QVideoFrame m_pendingFrame;
    QMutex m_frameMutex;
    int m_bytesPerLine;
    int m_referenceCount;
    bool m_flushing;
    bool m_frameProbed; // true if at least one frame was probed
};

QT_END_NAMESPACE

#endif // QGSTREAMERVIDEOPROBECONTROL_H
