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

class Video: public ouzel::scene::Component
{
public:
    Video();
    virtual ~Video();

    virtual bool init(const std::string& stream);

    virtual void update(float delta);
    virtual void draw(const ouzel::Matrix4& transformMatrix,
                      const ouzel::Color& drawColor,
                      ouzel::scene::Camera* camera) override;

protected:
    bool readFrame();

    ouzel::graphics::TexturePtr texture;
    ouzel::graphics::ShaderPtr shader;
    ouzel::graphics::BlendStatePtr blendState;
    ouzel::graphics::MeshBufferPtr meshBuffer;
    ouzel::graphics::IndexBufferPtr indexBuffer;
    ouzel::graphics::VertexBufferPtr vertexBuffer;

    ouzel::UpdateCallback updateCallback;

    int videoStream;
    AVFormatContext* formatCtx = nullptr;
    AVCodecContext* codecCtx = nullptr;
    AVCodec* codec = nullptr;
    AVFrame* frame = nullptr;
    struct SwsContext* scalerCtx = nullptr;

    std::queue<AVFrame*> frames;
    float sinceLastFrame = 0.0f;
};
