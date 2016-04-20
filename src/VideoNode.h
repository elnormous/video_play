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

class VideoNode: public ouzel::scene::Node
{
public:
    VideoNode();
    virtual ~VideoNode();

    virtual bool init();

    virtual void update(float delta);
    virtual void draw() override;

protected:
    bool readFrame();

    ouzel::graphics::TexturePtr _texture;
    ouzel::graphics::ShaderPtr _shader;
    ouzel::graphics::MeshBufferPtr _mesh;

    ouzel::UpdateCallbackPtr _updateCallback;

    int videoStream;
    AVFormatContext* _formatCtx = nullptr;
    AVCodecContext* _codecCtx = nullptr;
    AVCodec* _codec = nullptr;
    AVFrame* _frame = nullptr;
    struct SwsContext* _scalerCtx = nullptr;

    std::queue<AVFrame*> _frames;
    float _sinceLastFrame = 0.0f;
};
