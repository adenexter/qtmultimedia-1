/****************************************************************************
**
** Copyright (C) 2013 Jolla Ltd.
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
#include "qgstutils_p.h"

QT_BEGIN_NAMESPACE

QGstreamerBufferProbe::QGstreamerBufferProbe(Flags flags)
#if GST_CHECK_VERSION(1,0,0)
    : m_capsProbeId(-1)
#else
    : m_caps(0)
#endif
    , m_bufferProbeId(-1)
    , m_flags(flags)
{
}

QGstreamerBufferProbe::~QGstreamerBufferProbe()
{
}

void QGstreamerBufferProbe::addProbeToPad(GstPad *pad, bool downstream)
{
    if (GstCaps *caps = qt_gst_pad_get_current_caps(pad)) {
        probeCaps(caps);
        gst_caps_unref(caps);
    }
#if GST_CHECK_VERSION(1,0,0)
    if (m_flags & ProbeCaps) {
        m_capsProbeId = gst_pad_add_probe(
                    pad,
                    downstream
                        ? GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM
                        : GST_PAD_PROBE_TYPE_EVENT_UPSTREAM,
                    capsProbe,
                    this,
                    NULL);
    }
    if (m_flags & ProbeBuffers) {
        m_bufferProbeId = gst_pad_add_probe(
                    pad, GST_PAD_PROBE_TYPE_BUFFER, bufferProbe, this, NULL);
    }
#else
    Q_UNUSED(downstream);

    gst_pad_add_buffer_probe(pad, G_CALLBACK(bufferProbe), this);
#endif
}

void QGstreamerBufferProbe::removeProbeFromPad(GstPad *pad)
{
#if GST_CHECK_VERSION(1,0,0)
    if (m_capsProbeId != -1) {
        gst_pad_remove_probe(pad, m_capsProbeId);
        m_capsProbeId = -1;
    }
    if (m_bufferProbeId != -1) {
        gst_pad_remove_probe(pad, m_bufferProbeId);
        m_bufferProbeId = -1;
    }
#else
    if (m_bufferProbeId != -1) {
        gst_pad_remove_buffer_probe(pad, m_bufferProbeId);
        m_bufferProbeId = -1;
    }
#endif
}

void QGstreamerBufferProbe::probeCaps(GstCaps *)
{
}

bool QGstreamerBufferProbe::probeBuffer(GstBuffer *)
{
    return true;
}

#if GST_CHECK_VERSION(1,0,0)
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
    if (GstBuffer * const buffer = gst_pad_probe_info_get_buffer(info))
        return control->probeBuffer(buffer) ? GST_PAD_PROBE_OK : GST_PAD_PROBE_DROP;
    return GST_PAD_PROBE_OK;
}
#else
gboolean QGstreamerBufferProbe::bufferProbe(GstElement *, GstBuffer *buffer, gpointer user_data)
{
    QGstreamerBufferProbe * const control = static_cast<QGstreamerBufferProbe *>(user_data);

    if (control->m_flags & ProbeCaps) {
        GstCaps *caps = gst_buffer_get_caps(buffer);
        if (caps && (!control->m_caps || !gst_caps_is_equal(control->m_caps, caps))) {
            qSwap(caps, control->m_caps);
            control->probeCaps(control->m_caps);
        }
        if (caps)
            gst_caps_unref(caps);
    }

    if (control->m_flags & ProbeBuffers) {
        return control->probeBuffer(buffer) ? TRUE : FALSE;
    } else {
        return TRUE;
    }
}
#endif

QT_END_NAMESPACE
