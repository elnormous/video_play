//
//  native_play
//

#pragma once

#if defined(_MSC_VER)
#include <basetsd.h>
typedef SSIZE_T ssize_t;
#endif
#include <vlc/vlc.h>

class VideoLibvlc: public ouzel::scene::Component
{
public:
    VideoLibvlc();
    virtual ~VideoLibvlc();

    virtual bool init(const std::string& stream);

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

    void lock();
    void unlock();
    void resize(unsigned newWidth, unsigned newHeight);

    std::vector<uint8_t>& getBuffer() { return buffer; }

protected:
    std::shared_ptr<ouzel::graphics::Texture> texture;
    std::shared_ptr<ouzel::graphics::Shader> shader;
    std::shared_ptr<ouzel::graphics::BlendState> blendState;
    std::shared_ptr<ouzel::graphics::MeshBuffer> meshBuffer;
    std::shared_ptr<ouzel::graphics::Buffer> indexBuffer;
    std::shared_ptr<ouzel::graphics::Buffer> vertexBuffer;

    libvlc_instance_t* inst = nullptr;
    libvlc_media_player_t* mp = nullptr;
    libvlc_media_t* m = nullptr;

    unsigned width = 0;
    unsigned height = 0;

    std::mutex dataMutex;
    std::vector<uint8_t> buffer;
    std::atomic<bool> dirty;
};
