//
//  native_play
//

#include "VideoLibvlc.h"

#include <cstdio>
#include <cinttypes>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace ouzel;
using namespace scene;
using namespace graphics;

VideoLibvlc::VideoLibvlc()
{
}

VideoLibvlc::~VideoLibvlc()
{
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

    // TODO: load video
    texture = std::make_shared<ouzel::graphics::Texture>();
    texture->init(Size2(100, 100), true, false);

    updateCallback.callback = std::bind(&VideoLibvlc::update, this, std::placeholders::_1);
    sharedEngine->scheduleUpdate(&updateCallback);

    return true;
}

void VideoLibvlc::update(float delta)
{
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
