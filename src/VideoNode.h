//
//  native_play
//

#pragma once

extern "C"
{
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavformat/avio.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavcodec/avcodec.h>
}

class VideoNode: public ouzel::scene::Drawable
{
public:
    VideoNode();
    virtual ~VideoNode();

    virtual bool init();

    virtual void update(float delta);
    virtual void draw(const ouzel::Matrix4& projectionMatrix, const ouzel::Matrix4& transformMatrix, const ouzel::graphics::Color& drawColor) override;

protected:
    bool readFrame();

    ouzel::graphics::TexturePtr texture;
    ouzel::graphics::ShaderPtr shader;
    ouzel::graphics::MeshBufferPtr mesh;

    ouzel::UpdateCallbackPtr updateCallback;

    int videoStream;
    AVFormatContext* formatCtx = nullptr;
    AVCodecContext* codecCtx = nullptr;
    AVCodec* codec = nullptr;
    AVFrame* frame = nullptr;
    struct SwsContext* scalerCtx = nullptr;

    std::queue<AVFrame*> frames;
    float sinceLastFrame = 0.0f;
};
