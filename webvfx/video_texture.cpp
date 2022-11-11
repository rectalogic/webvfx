#include <QVideoFrame>
#include <QVideoFrameFormat>
#include <QVideoSink>

#include "webvfx/video_texture.h"
#include "webvfx/webvfx.h"

namespace WebVfx {

VideoTextureData::VideoTextureData()
    : videoSink(new QVideoSink(this))
{
    connect(videoSink, SIGNAL(videoFrameChanged(const QVideoFrame&)), SLOT(onVideoFrameChanged()));
}

void VideoTextureData::onVideoFrameChanged()
{
    QVideoFrame frame = videoSink->videoFrame();
    if (frame.pixelFormat() != QVideoFrameFormat::Format_RGBA8888) {
        log("Unable to handle QVideoFrame pixel format");
        return;
    }
    frame.map(QVideoFrame::ReadOnly);
    setSize(frame.size());
    setHasTransparency(true);
    setFormat(QQuick3DTextureData::RGBA8);
    setTextureData(QByteArray::fromRawData((const char*)frame.bits(0), frame.mappedBytes(0)));
    frame.unmap();
}
}