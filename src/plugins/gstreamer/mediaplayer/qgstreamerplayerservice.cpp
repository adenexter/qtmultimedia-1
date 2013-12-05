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

#include <QtCore/qvariant.h>
#include <QtCore/qdebug.h>

#if defined(HAVE_WIDGETS)
#include <QtWidgets/qwidget.h>
#endif

#include "qgstreamerplayerservice.h"
#include "qgstreamerplayercontrol.h"
#include "qgstreamerplayersession.h"
#include "qgstreamermetadataprovider.h"
#include "qgstreameravailabilitycontrol.h"

#if defined(HAVE_WIDGETS)
#include <private/qgstreamervideowidget_p.h>
#endif
#include <private/qgstreamervideowindow_p.h>
#include <private/qgstreamervideorenderer_p.h>
#include <private/qgstreamervideosinkcontrol_p.h>

#if defined(Q_WS_MAEMO_6) && defined(__arm__)
#include "private/qgstreamergltexturerenderer.h"
#endif

#if defined(HAVE_MIR) && defined (__arm__)
#include "private/qgstreamermirtexturerenderer_p.h"
#endif

#include "qgstreamerstreamscontrol.h"
#include <private/qgstreameraudioprobecontrol_p.h>
#include <private/qgstreamervideoprobecontrol_p.h>

#include <private/qmediaplaylistnavigator_p.h>
#include <qmediaplaylist.h>
#include <private/qmediaresourceset_p.h>

QT_BEGIN_NAMESPACE

QGstreamerPlayerService::QGstreamerPlayerService(QObject *parent):
     QMediaService(parent)
     , m_audioProbeControl(0)
     , m_videoProbeControl(0)
     , m_videoOutput(0)
     , m_videoRenderer(0)
     , m_videoWindow(0)
#if defined(HAVE_WIDGETS)
     , m_videoWidget(0)
#endif
     , m_videoReferenceCount(0)
{
    m_session = new QGstreamerPlayerSession(this);
    m_control = new QGstreamerPlayerControl(m_session, this);
    m_metaData = new QGstreamerMetaDataProvider(m_session, this);
    m_streamsControl = new QGstreamerStreamsControl(m_session,this);
    m_availabilityControl = new QGStreamerAvailabilityControl(m_control->resources(), this);

#if defined(Q_WS_MAEMO_6) && defined(__arm__)
    m_videoRenderer = new QGstreamerGLTextureRenderer(this);
#elif defined(HAVE_MIR) && defined (__arm__)
    m_videoRenderer = new QGstreamerMirTextureRenderer(this, m_session);
#else
    m_videoRenderer = new QGstreamerVideoRenderer(this);
#endif

#ifdef Q_WS_MAEMO_6
    m_videoWindow = new QGstreamerVideoWindow(this, "omapxvsink");
#else
    m_videoWindow = new QGstreamerVideoWindow(this);
#endif

#if defined(HAVE_WIDGETS)
    m_videoWidget = new QGstreamerVideoWidgetControl(this);
#endif
}

QGstreamerPlayerService::~QGstreamerPlayerService()
{
}

QMediaControl *QGstreamerPlayerService::requestControl(const char *name)
{
    if (qstrcmp(name,QMediaPlayerControl_iid) == 0)
        return m_control;

    if (qstrcmp(name,QMetaDataReaderControl_iid) == 0)
        return m_metaData;

    if (qstrcmp(name,QMediaStreamsControl_iid) == 0)
        return m_streamsControl;

    if (qstrcmp(name, QMediaAvailabilityControl_iid) == 0)
        return m_availabilityControl;

    if (qstrcmp(name, QMediaVideoProbeControl_iid) == 0) {
        if (!m_videoProbeControl) {
            increaseVideoRef();
            m_videoProbeControl = new QGstreamerVideoProbeControl(this);
            m_session->addProbe(m_videoProbeControl);
        }
        m_videoProbeControl->reference();
        return m_videoProbeControl;
    }

    if (qstrcmp(name, QMediaAudioProbeControl_iid) == 0) {
        if (!m_audioProbeControl) {
            m_audioProbeControl = new QGstreamerAudioProbeControl(this);
            m_session->addProbe(m_audioProbeControl);
        }
        m_audioProbeControl->reference();
        return m_audioProbeControl;
    }

    if (!m_videoOutput) {
        if (qstrcmp(name, QVideoRendererControl_iid) == 0)
            m_videoOutput = m_videoRenderer;
        else if (qstrcmp(name, QVideoWindowControl_iid) == 0)
            m_videoOutput = m_videoWindow;
        else if (qstrcmp(name, QGStreamerVideoSinkControl_iid) == 0)
            m_videoOutput = new QGStreamerVideoSinkControl(this);
#if defined(HAVE_WIDGETS)
        else if (qstrcmp(name, QVideoWidgetControl_iid) == 0)
            m_videoOutput = m_videoWidget;
#endif

        if (m_videoOutput) {
            increaseVideoRef();
            m_control->setVideoOutput(m_videoOutput);
            return m_videoOutput;
        }
    }

    return 0;
}

void QGstreamerPlayerService::releaseControl(QMediaControl *control)
{
    if (!control) {
        return;
    } else if (control == m_videoOutput) {
        m_videoOutput = 0;
        m_control->setVideoOutput(0);
        decreaseVideoRef();
    } else if (control == m_videoProbeControl && m_videoProbeControl->release()) {
        m_session->removeProbe(m_videoProbeControl);
        delete m_videoProbeControl;
        m_videoProbeControl = 0;
        decreaseVideoRef();
    } else if (control == m_audioProbeControl && m_audioProbeControl->release()) {
        m_session->removeProbe(m_audioProbeControl);
        delete m_audioProbeControl;
        m_audioProbeControl = 0;
    }
}

void QGstreamerPlayerService::increaseVideoRef()
{
    m_videoReferenceCount++;
    if (m_videoReferenceCount == 1) {
        m_control->resources()->setVideoEnabled(true);
    }
}

void QGstreamerPlayerService::decreaseVideoRef()
{
    m_videoReferenceCount--;
    if (m_videoReferenceCount == 0) {
        m_control->resources()->setVideoEnabled(false);
    }
}

QT_END_NAMESPACE
