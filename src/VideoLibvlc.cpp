//
//  native_play
//

#include "VideoLibvlc.h"

using namespace ouzel;
using namespace scene;
using namespace graphics;

const uint32_t WIDTH = 3840;
const uint32_t HEIGHT = 2160;

/*unsigned setup(void** opaque, char* chroma, unsigned* width, unsigned* height, unsigned* pitches, unsigned* lines)
{
    pixels.resize((*width) * (*height) * 4);
    
    return 1;
}*/

void cleanup(void* opaque)
{
}

static void* lock(void* opaque, void** p_pixels)
{
    VideoLibvlc* video = reinterpret_cast<VideoLibvlc*>(opaque);
    video->lock();
    *p_pixels = video->getBuffer().data();

    return nullptr; // picture id not needed
}

static void unlock(void* opaque, void* id, void* const* pixels)
{
    VideoLibvlc* video = reinterpret_cast<VideoLibvlc*>(opaque);
    video->unlock();

    assert(id == nullptr);
}

static void display(void *opaque, void *id)
{
    assert(id == nullptr);
}

VideoLibvlc::VideoLibvlc():
    dirty(false)
{
}

VideoLibvlc::~VideoLibvlc()
{
    if (m) libvlc_media_release(m);
    if (mp)
    {
        libvlc_media_player_stop(mp);
        libvlc_media_player_release(mp);
    }
    if (inst) libvlc_release(inst);
}

bool VideoLibvlc::init(const std::string& stream)
{
    shader = sharedEngine->getCache()->getShader(SHADER_TEXTURE);
    blendState = sharedEngine->getCache()->getBlendState(graphics::BLEND_ALPHA);

    std::vector<uint16_t> indices = {0, 1, 2, 1, 3, 2};

    Size2 size(192.0f, 108.0f);

    std::vector<VertexPCT> vertices = {
        VertexPCT(Vector3(-size.width() / 2.0f, -size.height() / 2.0f, 0.0f), Color(255, 255, 255, 255), Vector2(0.0f, 1.0f)),
        VertexPCT(Vector3(size.width() / 2.0f, -size.height() / 2.0f, 0.0f), Color(255, 255, 255, 255), Vector2(1.0f, 1.0f)),
        VertexPCT(Vector3(-size.width() / 2.0f, size.height() / 2.0f, 0.0f),  Color(255, 255, 255, 255), Vector2(0.0f, 0.0f)),
        VertexPCT(Vector3(size.width() / 2.0f, size.height() / 2.0f, 0.0f),  Color(255, 255, 255, 255), Vector2(1.0f, 0.0f))
    };

    boundingBox.set(Vector2(-size.width() / 2.0f, -size.height() / 2.0f),
                    Vector2(size.width() / 2.0f, size.height() / 2.0f));

    indexBuffer = std::make_shared<ouzel::graphics::Buffer>();
    indexBuffer->initFromBuffer(Buffer::Usage::INDEX, indices.data(), static_cast<uint32_t>(ouzel::getVectorSize(indices)), false);

    vertexBuffer = std::make_shared<ouzel::graphics::Buffer>();
    vertexBuffer->initFromBuffer(Buffer::Usage::VERTEX, vertices.data(), static_cast<uint32_t>(ouzel::getVectorSize(vertices)), true);

    meshBuffer = std::make_shared<ouzel::graphics::MeshBuffer>();
    meshBuffer->init(sizeof(uint16_t), indexBuffer, ouzel::graphics::VertexPCT::ATTRIBUTES, vertexBuffer);

    // Load the VLC engine
    inst = libvlc_new(0, nullptr);

    // Create a new item
    //m = libvlc_media_new_location(inst, stream.c_str());
    m = libvlc_media_new_path(inst, stream.c_str());

    // Create a media player playing environement
    mp = libvlc_media_player_new_from_media(m);

    // No need to keep the media now
    libvlc_media_release(m);
    m = nullptr;

    libvlc_video_set_callbacks(mp, ::lock, ::unlock, display, this);
    libvlc_video_set_format(mp, "RGBA", WIDTH, HEIGHT, WIDTH * 4);
    //libvlc_video_set_format_callbacks(mp, setup, cleanup);

    buffer.resize(WIDTH * HEIGHT * 4);

    texture = std::make_shared<ouzel::graphics::Texture>();
    texture->init(Size2(static_cast<float>(WIDTH), static_cast<float>(HEIGHT)), true, false);

    libvlc_media_player_play(mp);

    return true;
}

void VideoLibvlc::draw(const ouzel::Matrix4& transformMatrix,
                       const ouzel::Color& drawColor,
                       const ouzel::Matrix4& renderViewProjection,
                       const std::shared_ptr<ouzel::graphics::Texture>& renderTarget,
                       const ouzel::Rectangle& renderViewport,
                       bool depthWrite,
                       bool depthTest,
                       bool wireframe,
                       bool scissorTest,
                       const ouzel::Rectangle& scissorRectangle)
{
    Component::draw(transformMatrix,
                    drawColor,
                    renderViewProjection,
                    renderTarget,
                    renderViewport,
                    depthWrite,
                    depthTest,
                    wireframe,
                    scissorTest,
                    scissorRectangle);

    if (texture)
    {
        if (dirty)
        {
            std::lock_guard<std::mutex> dataLock(dataMutex);
            texture->setData(buffer, Size2(static_cast<float>(WIDTH), static_cast<float>(HEIGHT)));
            dirty = false;
        }

        Matrix4 modelViewProj = renderViewProjection * transformMatrix;
        float colorVector[] = {drawColor.normR(), drawColor.normG(), drawColor.normB(), drawColor.normA()};

        std::vector<std::vector<float>> pixelShaderConstants(1);
        pixelShaderConstants[0] = {std::begin(colorVector), std::end(colorVector)};

        std::vector<std::vector<float>> vertexShaderConstants(1);
        vertexShaderConstants[0] = {std::begin(modelViewProj.m), std::end(modelViewProj.m)};

        sharedEngine->getRenderer()->addDrawCommand({texture},
                                                    shader,
                                                    pixelShaderConstants,
                                                    vertexShaderConstants,
                                                    blendState,
                                                    meshBuffer,
                                                    0,
                                                    graphics::Renderer::DrawMode::TRIANGLE_LIST,
                                                    0,
                                                    renderTarget,
                                                    renderViewport,
                                                    depthWrite,
                                                    depthTest,
                                                    wireframe,
                                                    scissorTest,
                                                    scissorRectangle);
    }
}

void VideoLibvlc::lock()
{
    dataMutex.lock();
}

void VideoLibvlc::unlock()
{
    dataMutex.unlock();
    dirty = true;
}
