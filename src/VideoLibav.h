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

class VideoLibav: public ouzel::scene::Component
{
public:
    VideoLibav();
    virtual ~VideoLibav();

    virtual bool init(const std::string& stream);

    virtual void update(float delta);
    virtual void draw(const ouzel::Matrix4& transformMatrix,
                      const ouzel::Color& drawColor,
                      const ouzel::Matrix4& renderViewProjection,
                      const std::shared_ptr<ouzel::graphics::Texture>& renderTarget,
                      const ouzel::Rectangle& renderViewport,
                      bool depthWrite,
                      bool depthTest,
                      bool wireframe,
                      bool scissorTest,
                      const ouzel::Rectangle& scissorRectangle) override;

protected:
    bool readFrame();

    std::shared_ptr<ouzel::graphics::Texture> texture;
    std::shared_ptr<ouzel::graphics::Shader> shader;
    std::shared_ptr<ouzel::graphics::BlendState> blendState;
    std::shared_ptr<ouzel::graphics::MeshBuffer> meshBuffer;
    std::shared_ptr<ouzel::graphics::Buffer> indexBuffer;
    std::shared_ptr<ouzel::graphics::Buffer> vertexBuffer;

    ouzel::UpdateCallback updateCallback;

    int videoStream;
    AVFormatContext* formatCtx = nullptr;
    AVCodecContext* codecCtx = nullptr;
    AVCodec* codec = nullptr;
    AVFrame* frame = nullptr;
    struct SwsContext* scalerCtx = nullptr;

    std::queue<AVFrame*> frames;
};
