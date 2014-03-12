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

#include "qgstutils_p.h"

#include <QtCore/qdatetime.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qvariant.h>
#include <QtCore/qsize.h>
#include <QtCore/qset.h>
#include <QtCore/qstringlist.h>
#include <QtGui/qimage.h>
#include <qaudioformat.h>
#include <QtMultimedia/qvideosurfaceformat.h>

#include <gst/audio/audio.h>
#include <gst/video/video.h>

template<typename T, int N> static int lengthOf(const T (&)[N]) { return N; }

QT_BEGIN_NAMESPACE

//internal
static void addTagToMap(const GstTagList *list,
                        const gchar *tag,
                        gpointer user_data)
{
    QMap<QByteArray, QVariant> *map = reinterpret_cast<QMap<QByteArray, QVariant>* >(user_data);

    GValue val;
    val.g_type = 0;
    gst_tag_list_copy_value(&val,list,tag);

    switch( G_VALUE_TYPE(&val) ) {
        case G_TYPE_STRING:
        {
            const gchar *str_value = g_value_get_string(&val);
            map->insert(QByteArray(tag), QString::fromUtf8(str_value));
            break;
        }
        case G_TYPE_INT:
            map->insert(QByteArray(tag), g_value_get_int(&val));
            break;
        case G_TYPE_UINT:
            map->insert(QByteArray(tag), g_value_get_uint(&val));
            break;
        case G_TYPE_LONG:
            map->insert(QByteArray(tag), qint64(g_value_get_long(&val)));
            break;
        case G_TYPE_BOOLEAN:
            map->insert(QByteArray(tag), g_value_get_boolean(&val));
            break;
        case G_TYPE_CHAR:
            map->insert(QByteArray(tag), g_value_get_schar(&val));
            break;
        case G_TYPE_DOUBLE:
            map->insert(QByteArray(tag), g_value_get_double(&val));
            break;
        default:
            // GST_TYPE_DATE is a function, not a constant, so pull it out of the switch
            if (G_VALUE_TYPE(&val) == G_TYPE_DATE) {
                const GDate *date = static_cast<GDate *>(g_value_get_boxed(&val));
                if (g_date_valid(date)) {
                    int year = g_date_get_year(date);
                    int month = g_date_get_month(date);
                    int day = g_date_get_day(date);
                    map->insert(QByteArray(tag), QDate(year,month,day));
                    if (!map->contains("year"))
                        map->insert("year", year);
                }
            } else if (G_VALUE_TYPE(&val) == GST_TYPE_FRACTION) {
                int nom = gst_value_get_fraction_numerator(&val);
                int denom = gst_value_get_fraction_denominator(&val);

                if (denom > 0) {
                    map->insert(QByteArray(tag), double(nom)/denom);
                }
            }
            break;
    }

    g_value_unset(&val);
}

/*!
  Convert GstTagList structure to QMap<QByteArray, QVariant>.

  Mapping to int, bool, char, string, fractions and date are supported.
  Fraction values are converted to doubles.
*/
QMap<QByteArray, QVariant> QGstUtils::gstTagListToMap(const GstTagList *tags)
{
    QMap<QByteArray, QVariant> res;
    gst_tag_list_foreach(tags, addTagToMap, &res);

    return res;
}

/*!
  Returns resolution of \a caps.
  If caps doesn't have a valid size, and ampty QSize is returned.
*/
QSize QGstUtils::capsResolution(const GstCaps *caps)
{
    QSize size;

    if (caps) {
        const GstStructure *structure = gst_caps_get_structure(caps, 0);
        gst_structure_get_int(structure, "width", &size.rwidth());
        gst_structure_get_int(structure, "height", &size.rheight());
    }

    return size;
}

/*!
  Returns aspect ratio corrected resolution of \a caps.
  If caps doesn't have a valid size, an empty QSize is returned.
*/
QSize QGstUtils::capsCorrectedResolution(const GstCaps *caps)
{
    QSize size;

    if (caps) {
        const GstStructure *structure = gst_caps_get_structure(caps, 0);
        gst_structure_get_int(structure, "width", &size.rwidth());
        gst_structure_get_int(structure, "height", &size.rheight());

        gint aspectNum = 0;
        gint aspectDenum = 0;
        if (!size.isEmpty() && gst_structure_get_fraction(
                    structure, "pixel-aspect-ratio", &aspectNum, &aspectDenum)) {
            if (aspectDenum > 0)
                size.setWidth(size.width()*aspectNum/aspectDenum);
        }
    }

    return size;
}


namespace {

struct AudioFormat
{
    GstAudioFormat format;
    QAudioFormat::SampleType sampleType;
    QAudioFormat::Endian byteOrder;
    int sampleSize;
};
static const AudioFormat qt_audioLookup[] =
{
    { GST_AUDIO_FORMAT_S8   , QAudioFormat::SignedInt  , QAudioFormat::LittleEndian, 8  },
    { GST_AUDIO_FORMAT_U8   , QAudioFormat::UnSignedInt, QAudioFormat::LittleEndian, 8  },
    { GST_AUDIO_FORMAT_S16LE, QAudioFormat::SignedInt  , QAudioFormat::LittleEndian, 16 },
    { GST_AUDIO_FORMAT_S16BE, QAudioFormat::SignedInt  , QAudioFormat::BigEndian   , 16 },
    { GST_AUDIO_FORMAT_U16LE, QAudioFormat::UnSignedInt, QAudioFormat::LittleEndian, 16 },
    { GST_AUDIO_FORMAT_U16BE, QAudioFormat::UnSignedInt, QAudioFormat::BigEndian   , 16 },
    { GST_AUDIO_FORMAT_S32LE, QAudioFormat::SignedInt  , QAudioFormat::LittleEndian, 32 },
    { GST_AUDIO_FORMAT_S32BE, QAudioFormat::SignedInt  , QAudioFormat::BigEndian   , 32 },
    { GST_AUDIO_FORMAT_U32LE, QAudioFormat::UnSignedInt, QAudioFormat::LittleEndian, 32 },
    { GST_AUDIO_FORMAT_U32BE, QAudioFormat::UnSignedInt, QAudioFormat::BigEndian   , 32 },
    { GST_AUDIO_FORMAT_S24LE, QAudioFormat::SignedInt  , QAudioFormat::LittleEndian, 24 },
    { GST_AUDIO_FORMAT_S24BE, QAudioFormat::SignedInt  , QAudioFormat::BigEndian   , 24 },
    { GST_AUDIO_FORMAT_U24LE, QAudioFormat::UnSignedInt, QAudioFormat::LittleEndian, 24 },
    { GST_AUDIO_FORMAT_U24BE, QAudioFormat::UnSignedInt, QAudioFormat::BigEndian   , 24 },
    { GST_AUDIO_FORMAT_F32LE, QAudioFormat::Float      , QAudioFormat::LittleEndian, 32 },
    { GST_AUDIO_FORMAT_F32BE, QAudioFormat::Float      , QAudioFormat::BigEndian   , 32 },
    { GST_AUDIO_FORMAT_F64LE, QAudioFormat::Float      , QAudioFormat::LittleEndian, 64 },
    { GST_AUDIO_FORMAT_F64BE, QAudioFormat::Float      , QAudioFormat::BigEndian   , 64 }
};

}

/*!
  Returns audio format for caps.
  If caps doesn't have a valid audio format, an empty QAudioFormat is returned.
*/

QAudioFormat QGstUtils::audioFormatForCaps(const GstCaps *caps)
{
    QAudioFormat format;
    GstAudioInfo info;
    if (gst_audio_info_from_caps(&info, caps)) {
        for (int i = 0; i < lengthOf(qt_audioLookup); ++i) {
            if (qt_audioLookup[i].format != info.finfo->format)
                continue;

            format.setSampleType(qt_audioLookup[i].sampleType);
            format.setByteOrder(qt_audioLookup[i].byteOrder);
            format.setSampleSize(qt_audioLookup[i].sampleSize);
            format.setSampleRate(info.rate);
            format.setChannelCount(info.channels);
            format.setCodec(QStringLiteral("audio/pcm"));

            return format;
        }
    }
    return format;
}


/*!
  Returns audio format for a sample.
  If the buffer doesn't have a valid audio format, an empty QAudioFormat is returned.
*/

QAudioFormat QGstUtils::audioFormatForSample(GstSample *sample)
{
    GstCaps* caps = gst_sample_get_caps(sample);
    if (!caps)
        return QAudioFormat();

    return QGstUtils::audioFormatForCaps(caps);
}


/*!
  Builds GstCaps for an audio format.
  Returns 0 if the audio format is not valid.
  Caller must unref GstCaps.
*/

GstCaps *QGstUtils::capsForAudioFormat(QAudioFormat format)
{
    if (!format.isValid())
        return 0;

    const QAudioFormat::SampleType sampleType = format.sampleType();
    const QAudioFormat::Endian byteOrder = format.byteOrder();
    const int sampleSize = format.sampleSize();

    for (int i = 0; i < lengthOf(qt_audioLookup); ++i) {
        if (qt_audioLookup[i].sampleType != sampleType
                || qt_audioLookup[i].byteOrder != byteOrder
                || qt_audioLookup[i].sampleSize != sampleSize) {
            continue;
        }

        return gst_caps_new_simple(
                    "audio/x-raw",
                    "format"  , G_TYPE_STRING, gst_audio_format_to_string(qt_audioLookup[i].format),
                    "rate"    , G_TYPE_INT   , format.sampleRate(),
                    "channels", G_TYPE_INT   , format.channelCount(),
                    NULL);
    }
    return 0;
}

void QGstUtils::initializeGst()
{
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        gst_init(NULL, NULL);
    }
}

namespace {
    const char* getCodecAlias(const QString &codec)
    {
        if (codec.startsWith("avc1."))
            return "video/x-h264";

        if (codec.startsWith("mp4a."))
            return "audio/mpeg4";

        if (codec.startsWith("mp4v.20."))
            return "video/mpeg4";

        if (codec == "samr")
            return "audio/amr";

        return 0;
    }

    const char* getMimeTypeAlias(const QString &mimeType)
    {
        if (mimeType == "video/mp4")
            return "video/mpeg4";

        if (mimeType == "audio/mp4")
            return "audio/mpeg4";

        if (mimeType == "video/ogg"
            || mimeType == "audio/ogg")
            return "application/ogg";

        return 0;
    }
}

QMultimedia::SupportEstimate QGstUtils::hasSupport(const QString &mimeType,
                                                    const QStringList &codecs,
                                                    const QSet<QString> &supportedMimeTypeSet)
{
    if (supportedMimeTypeSet.isEmpty())
        return QMultimedia::NotSupported;

    QString mimeTypeLowcase = mimeType.toLower();
    bool containsMimeType = supportedMimeTypeSet.contains(mimeTypeLowcase);
    if (!containsMimeType) {
        const char* mimeTypeAlias = getMimeTypeAlias(mimeTypeLowcase);
        containsMimeType = supportedMimeTypeSet.contains(mimeTypeAlias);
        if (!containsMimeType) {
            containsMimeType = supportedMimeTypeSet.contains("video/" + mimeTypeLowcase)
                               || supportedMimeTypeSet.contains("video/x-" + mimeTypeLowcase)
                               || supportedMimeTypeSet.contains("audio/" + mimeTypeLowcase)
                               || supportedMimeTypeSet.contains("audio/x-" + mimeTypeLowcase);
        }
    }

    int supportedCodecCount = 0;
    foreach (const QString &codec, codecs) {
        QString codecLowcase = codec.toLower();
        const char* codecAlias = getCodecAlias(codecLowcase);
        if (codecAlias) {
            if (supportedMimeTypeSet.contains(codecAlias))
                supportedCodecCount++;
        } else if (supportedMimeTypeSet.contains("video/" + codecLowcase)
                   || supportedMimeTypeSet.contains("video/x-" + codecLowcase)
                   || supportedMimeTypeSet.contains("audio/" + codecLowcase)
                   || supportedMimeTypeSet.contains("audio/x-" + codecLowcase)) {
            supportedCodecCount++;
        }
    }
    if (supportedCodecCount > 0 && supportedCodecCount == codecs.size())
        return QMultimedia::ProbablySupported;

    if (supportedCodecCount == 0 && !containsMimeType)
        return QMultimedia::NotSupported;

    return QMultimedia::MaybeSupported;
}

QSet<QString> QGstUtils::supportedMimeTypes(bool (*isValidFactory)(GstElementFactory *factory))
{
    QSet<QString> supportedMimeTypes;

    //enumerate supported mime types
    gst_init(NULL, NULL);

    GstRegistry * const registry = gst_registry_get();
    GList *orig_plugins = gst_registry_get_plugin_list(registry);
    for  (GList *plugins = orig_plugins; plugins; plugins = g_list_next(plugins)) {
        GstPlugin *plugin = (GstPlugin *) (plugins->data);
        if (GST_OBJECT_FLAG_IS_SET(GST_OBJECT(plugin), GST_PLUGIN_FLAG_BLACKLISTED))
            continue;

        GList *orig_features = gst_registry_get_feature_list_by_plugin(
                    registry, gst_plugin_get_name(plugin));
        for (GList *features = orig_features; features; features = g_list_next(features)) {
            if (G_UNLIKELY(features->data == NULL))
                continue;

            GstPluginFeature *feature = GST_PLUGIN_FEATURE(features->data);
            GstElementFactory *factory;

            if (GST_IS_TYPE_FIND_FACTORY(feature)) {
                QString name(gst_plugin_feature_get_name(feature));
                if (name.contains('/')) //filter out any string without '/' which is obviously not a mime type
                    supportedMimeTypes.insert(name.toLower());
                continue;
            } else if (!GST_IS_ELEMENT_FACTORY (feature)
                        || !(factory = GST_ELEMENT_FACTORY(gst_plugin_feature_load(feature)))) {
                continue;
            } else if (isValidFactory(factory)) {
                // Do nothing
            } else for (const GList *pads = gst_element_factory_get_static_pad_templates(factory);
                        pads;
                        pads = g_list_next(pads)) {
                GstStaticPadTemplate *padtemplate = static_cast<GstStaticPadTemplate *>(pads->data);

                if (padtemplate->direction == GST_PAD_SINK && padtemplate->static_caps.string) {
                    GstCaps *caps = gst_static_caps_get(&padtemplate->static_caps);
                    if (gst_caps_is_any(caps) || gst_caps_is_empty(caps)) {
                    } else for (guint i = 0; i < gst_caps_get_size(caps); i++) {
                        GstStructure *structure = gst_caps_get_structure(caps, i);
                        QString nameLowcase = QString(gst_structure_get_name(structure)).toLower();

                        supportedMimeTypes.insert(nameLowcase);
                        if (nameLowcase.contains("mpeg")) {
                            //Because mpeg version number is only included in the detail
                            //description,  it is necessary to manually extract this information
                            //in order to match the mime type of mpeg4.
                            const GValue *value = gst_structure_get_value(structure, "mpegversion");
                            if (value) {
                                gchar *str = gst_value_serialize(value);
                                QString versions(str);
                                QStringList elements = versions.split(QRegExp("\\D+"), QString::SkipEmptyParts);
                                foreach (const QString &e, elements)
                                    supportedMimeTypes.insert(nameLowcase + e);
                                g_free(str);
                            }
                        }
                    }
                }
            }
            gst_object_unref(factory);
        }
        gst_plugin_feature_list_free(orig_features);
    }
    gst_plugin_list_free (orig_plugins);

#if defined QT_SUPPORTEDMIMETYPES_DEBUG
    QStringList list = supportedMimeTypes.toList();
    list.sort();
    if (qgetenv("QT_DEBUG_PLUGINS").toInt() > 0) {
        foreach (const QString &type, list)
            qDebug() << type;
    }
#endif
    return supportedMimeTypes;
}

namespace {

struct ColorFormat { QImage::Format imageFormat; GstVideoFormat gstFormat; };
static const ColorFormat qt_colorLookup[] =
{
    { QImage::Format_RGB32 ,  GST_VIDEO_FORMAT_xRGB  },
    { QImage::Format_ARGB32,  GST_VIDEO_FORMAT_ARGB  },
    { QImage::Format_RGB888,  GST_VIDEO_FORMAT_RGB   },
    { QImage::Format_RGB16 ,  GST_VIDEO_FORMAT_RGB16 }
};

}

QImage QGstUtils::bufferToImage(GstBuffer *buffer, const GstVideoInfo &videoInfo)
{
    QImage img;

    GstMapInfo mapInfo;
    if (!gst_buffer_map(buffer, &mapInfo, GST_MAP_READ))
        return img;

    if (videoInfo.finfo->format == GST_VIDEO_FORMAT_I420) {
        const int width = videoInfo.width;
        const int height = videoInfo.height;
        const uchar * const data = mapInfo.data;

        img = QImage(width/2, height/2, QImage::Format_RGB32);

        for (int y=0; y<height; y+=2) {
            const uchar *yLine = data + y*width;
            const uchar *uLine = data + width*height + y*width/4;
            const uchar *vLine = data + width*height*5/4 + y*width/4;

            for (int x=0; x<width; x+=2) {
                const qreal Y = 1.164*(yLine[x]-16);
                const int U = uLine[x/2]-128;
                const int V = vLine[x/2]-128;

                int b = qBound(0, int(Y + 2.018*U), 255);
                int g = qBound(0, int(Y - 0.813*V - 0.391*U), 255);
                int r = qBound(0, int(Y + 1.596*V), 255);

                img.setPixel(x/2,y/2,qRgb(r,g,b));
            }
        }
    } else for (int i = 0; i < lengthOf(qt_colorLookup); ++i) {
        if (qt_colorLookup[i].gstFormat != videoInfo.finfo->format)
            continue;

        const QImage image(
                    static_cast<const uchar *>(mapInfo.data),
                    videoInfo.width,
                    videoInfo.height,
                    videoInfo.stride[0],
                    qt_colorLookup[i].imageFormat);
        img = image;
        img.detach();

        break;
    }

    gst_buffer_unmap(buffer, &mapInfo);

    return img;
}


namespace {

struct VideoFormat
{
    QVideoFrame::PixelFormat pixelFormat;
    GstVideoFormat gstFormat;
};

static const VideoFormat qt_videoFormatLookup[] =
{
    { QVideoFrame::Format_YUV420P, GST_VIDEO_FORMAT_I420 },
    { QVideoFrame::Format_YV12   , GST_VIDEO_FORMAT_YV12 },
    { QVideoFrame::Format_UYVY   , GST_VIDEO_FORMAT_UYVY },
    { QVideoFrame::Format_YUYV   , GST_VIDEO_FORMAT_YUY2 },
    { QVideoFrame::Format_NV12   , GST_VIDEO_FORMAT_NV12 },
    { QVideoFrame::Format_NV21   , GST_VIDEO_FORMAT_NV21 },
    { QVideoFrame::Format_AYUV444, GST_VIDEO_FORMAT_AYUV },
    { QVideoFrame::Format_RGB32 ,  GST_VIDEO_FORMAT_BGRx },
    { QVideoFrame::Format_BGR32 ,  GST_VIDEO_FORMAT_RGBx }, // Beware endianess.
    { QVideoFrame::Format_ARGB32,  GST_VIDEO_FORMAT_ARGB },
    { QVideoFrame::Format_BGRA32,  GST_VIDEO_FORMAT_BGRA },
    { QVideoFrame::Format_RGB24 ,  GST_VIDEO_FORMAT_RGB },
    { QVideoFrame::Format_BGR24 ,  GST_VIDEO_FORMAT_BGR },
    { QVideoFrame::Format_RGB565,  GST_VIDEO_FORMAT_RGB16 }
};

static int indexOfVideoFormat(QVideoFrame::PixelFormat format)
{
    for (int i = 0; i < lengthOf(qt_videoFormatLookup); ++i)
        if (qt_videoFormatLookup[i].pixelFormat == format)
            return i;

    return -1;
}

static int indexOfVideoFormat(GstVideoFormat format)
{
    for (int i = 0; i < lengthOf(qt_videoFormatLookup); ++i)
        if (qt_videoFormatLookup[i].gstFormat == format)
            return i;

    return -1;
}

}

QVideoSurfaceFormat QGstUtils::formatForCaps(
        GstCaps *caps, int *bytesPerLine, QAbstractVideoBuffer::HandleType handleType)
{
    GstVideoInfo info;
    if (gst_video_info_from_caps(&info, caps)) {
        int index = indexOfVideoFormat(info.finfo->format);

        if (index != -1) {
            QVideoSurfaceFormat format(
                        QSize(info.width, info.height),
                        qt_videoFormatLookup[index].pixelFormat,
                        handleType);

            if (info.fps_d > 0)
                format.setFrameRate(qreal(info.fps_d) / info.fps_n);

            if (info.par_d > 0)
                format.setPixelAspectRatio(info.par_n, info.par_d);

            if (bytesPerLine)
                *bytesPerLine = info.stride[0];

            return format;
        }
    }

    return QVideoSurfaceFormat();
}

GstCaps *QGstUtils::capsForFormats(const QList<QVideoFrame::PixelFormat> &formats)
{
    GstCaps *caps = gst_caps_new_empty();

    foreach (QVideoFrame::PixelFormat format, formats) {
        int index = indexOfVideoFormat(format);

        if (index != -1) {
            gst_caps_append_structure(caps, gst_structure_new(
                    "video/x-raw",
                    "framerate", GST_TYPE_FRACTION_RANGE, 0, 1, INT_MAX, 1,
                    "width"    , GST_TYPE_INT_RANGE, 1, INT_MAX,
                    "height"   , GST_TYPE_INT_RANGE, 1, INT_MAX,
                    "format"   , G_TYPE_STRING, gst_video_format_to_string(qt_videoFormatLookup[index].gstFormat),
                    NULL));
        }
    }

    return caps;
}

void QGstUtils::setFrameTimeStamps(QVideoFrame *frame, GstBuffer *buffer)
{
    // GStreamer uses nanoseconds, Qt uses microseconds
    qint64 startTime = GST_BUFFER_TIMESTAMP(buffer);
    if (startTime >= 0) {
        frame->setStartTime(startTime/G_GINT64_CONSTANT (1000));

        qint64 duration = GST_BUFFER_DURATION(buffer);
        if (duration >= 0)
            frame->setEndTime((startTime + duration)/G_GINT64_CONSTANT (1000));
    }
}


QDebug operator <<(QDebug debug, GstCaps *caps)
{
    if (caps) {
        gchar *string = gst_caps_to_string(caps);
        debug = debug << string;
        g_free(string);
    }
    return debug;
}

QT_END_NAMESPACE
