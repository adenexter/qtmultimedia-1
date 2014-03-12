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

#ifndef QGSTBUFFERPOOLINTERFACE_P_H
#define QGSTBUFFERPOOLINTERFACE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qabstractvideobuffer.h>
#include <qvideosurfaceformat.h>
#include <QtCore/qobject.h>
#include <QtCore/qplugin.h>

#include <gst/gst.h>

QT_BEGIN_NAMESPACE

class QAbstractVideoSurface;

const QLatin1String QGstBufferPoolPluginKey("bufferpool");

class QGstBufferPool
{
public:
    virtual ~QGstBufferPool() {}

    virtual GstCaps *getCaps(QAbstractVideoSurface *surface) = 0;
    virtual bool start(QAbstractVideoSurface *surface, GstCaps *caps) = 0;
    virtual void stop(QAbstractVideoSurface *surface) = 0;  // surface may be null if unexpectedly deleted.
    virtual bool proposeAllocation(GstQuery *query) = 0;    // may be called from a thread.

    virtual bool present(QAbstractVideoSurface *surface, GstBuffer *buffer) = 0;
    virtual void flush(QAbstractVideoSurface *surface) = 0; // surface may be null if unexpectedly deleted.
};

/*!
    Abstract interface for video buffers allocation.
*/
class QGstBufferPoolInterface
{
public:
    virtual ~QGstBufferPoolInterface() {}

    virtual QGstBufferPool *createPool() = 0;
};

#define QGstBufferPoolInterface_iid "org.qt-project.qt.gstbufferpool/5.0"
Q_DECLARE_INTERFACE(QGstBufferPoolInterface, QGstBufferPoolInterface_iid)

class QGstBufferPoolPlugin : public QObject, public QGstBufferPoolInterface
{
    Q_OBJECT
    Q_INTERFACES(QGstBufferPoolInterface)
public:
    explicit QGstBufferPoolPlugin(QObject *parent = 0);
    virtual ~QGstBufferPoolPlugin() {}

    virtual QGstBufferPool *createPool() = 0;

};

QT_END_NAMESPACE

#endif
