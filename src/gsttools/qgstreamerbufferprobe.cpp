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

#include "qgstreamerbufferprobe_p.h"

QT_BEGIN_NAMESPACE

QGstreamerBufferProbe::QGstreamerBufferProbe()
    : m_capsProbeId(-1)
    , m_bufferProbeId(-1)
{
}

QGstreamerBufferProbe::~QGstreamerBufferProbe()
{
}

void QGstreamerBufferProbe::addProbe(GstPad *pad, bool downstream)
{
    if (GstCaps *caps = gst_pad_get_current_caps(pad)) {
        probeCaps(caps);
        gst_caps_unref(caps);
    }
    m_capsProbeId = gst_pad_add_probe(
                pad,
                downstream ? GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM : GST_PAD_PROBE_TYPE_EVENT_UPSTREAM,
                capsProbe,
                this,
                NULL);
    m_bufferProbeId = gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER, bufferProbe, this, NULL);
}

void QGstreamerBufferProbe::removeProbe(GstPad *pad)
{
    if (m_capsProbeId != -1) {
        gst_pad_remove_probe(pad, m_capsProbeId);
        m_capsProbeId = -1;
    }
    if (m_bufferProbeId != -1) {
        gst_pad_remove_probe(pad, m_bufferProbeId);
        m_bufferProbeId = -1;
    }
}

GstPadProbeReturn QGstreamerBufferProbe::capsProbe(
        GstPad *, GstPadProbeInfo *info, gpointer user_data)
{
    QGstreamerBufferProbe * const control = static_cast<QGstreamerBufferProbe *>(user_data);

    if (GstEvent * const event = gst_pad_probe_info_get_event(info)) {
        if (GST_EVENT_TYPE(event) == GST_EVENT_CAPS) {
            GstCaps *caps;
            gst_event_parse_caps(event, &caps);

            control->probeCaps(caps);
        }
    }
    return GST_PAD_PROBE_OK;
}

GstPadProbeReturn QGstreamerBufferProbe::bufferProbe(
        GstPad *, GstPadProbeInfo *info, gpointer user_data)
{
    QGstreamerBufferProbe * const control = static_cast<QGstreamerBufferProbe *>(user_data);
    if (GstBuffer * const buffer = gst_pad_probe_info_get_buffer(info)) {
        return control->probeBuffer(buffer);
    }
    return GST_PAD_PROBE_OK;
}

QT_END_NAMESPACE
