//
//  native_play
//

#include "VideoTextureNode.h"

using namespace ouzel;

VideoTextureNode::VideoTextureNode()
{
}

VideoTextureNode::~VideoTextureNode()
{
    sharedEngine->unscheduleUpdate(updateCallback);
}

bool VideoTextureNode::init()
{
    updateCallback = std::make_shared<UpdateCallback>();
    updateCallback->callback = std::bind(&VideoTextureNode::update, this, std::placeholders::_1);

    sharedEngine->scheduleUpdate(updateCallback);

    shader = sharedEngine->getCache()->getShader(SHADER_TEXTURE);

    std::vector<uint16_t> indices = {0, 1, 2, 1, 3, 2};

    std::vector<VertexPCT> vertices = {
        VertexPCT(Vector3(-1.0f, -1.0f, 0.0f), Color(255, 255, 255, 255), Vector2(0.0f, 1.0f)),
        VertexPCT(Vector3(1.0f, -1.0f, 0.0f), Color(255, 255, 255, 255), Vector2(1.0f, 1.0f)),
        VertexPCT(Vector3(-1.0f, 1.0f, 0.0f),  Color(255, 255, 255, 255), Vector2(0.0f, 0.0f)),
        VertexPCT(Vector3(1.0f, 1.0f, 0.0f),  Color(255, 255, 255, 255), Vector2(1.0f, 0.0f))
    };

    mesh = sharedEngine->getRenderer()->createMeshBuffer(indices.data(), sizeof(uint16_t), static_cast<uint32_t>(indices.size()), false,
                                                                   vertices.data(), sizeof(VertexPCT), static_cast<uint32_t>(vertices.size()), true,
                                                                   VertexPCT::ATTRIBUTES);

    texture = sharedEngine->getRenderer()->loadVideoTextureFromFile("/Users/elviss/Desktop/video/test.mov");

    return true;
}

void VideoTextureNode::update(float delta)
{

}

void VideoTextureNode::draw(const ouzel::Matrix4& projectionMatrix, const ouzel::Matrix4& transformMatrix, const ouzel::graphics::Color& drawColor)
{
    sharedEngine->getRenderer()->activateTexture(texture, 0);
    sharedEngine->getRenderer()->activateShader(shader);

    Matrix4 modelViewProj = projectionMatrix * transformMatrix;

    float colorVector[] = { drawColor.getR(), drawColor.getG(), drawColor.getB(), drawColor.getA() };

    shader->setVertexShaderConstant(0, sizeof(Matrix4), 1, modelViewProj.m);
    shader->setPixelShaderConstant(0, sizeof(colorVector), 1, colorVector);

    sharedEngine->getRenderer()->drawMeshBuffer(mesh);
}
