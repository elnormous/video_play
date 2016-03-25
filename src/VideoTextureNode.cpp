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
    Engine::getInstance()->unscheduleUpdate(_updateCallback);
}

bool VideoTextureNode::init()
{
    _updateCallback = std::make_shared<UpdateCallback>();
    _updateCallback->callback = std::bind(&VideoTextureNode::update, this, std::placeholders::_1);

    Engine::getInstance()->scheduleUpdate(_updateCallback);

    _shader = Engine::getInstance()->getCache()->getShader(SHADER_TEXTURE);

#ifdef OUZEL_PLATFORM_WINDOWS
    _uniModelViewProj = 0;
#else
    _uniModelViewProj = _shader->getVertexShaderConstantId("modelViewProj");
#endif

    std::vector<uint16_t> indices = {0, 1, 2, 1, 3, 2};

    std::vector<VertexPCT> vertices = {
        VertexPCT(Vector3(-1.0f, -1.0f, 0.0f), Color(255, 255, 255, 255), Vector2(0.0f, 1.0f)),
        VertexPCT(Vector3(1.0f, -1.0f, 0.0f), Color(255, 255, 255, 255), Vector2(1.0f, 1.0f)),
        VertexPCT(Vector3(-1.0f, 1.0f, 0.0f),  Color(255, 255, 255, 255), Vector2(0.0f, 0.0f)),
        VertexPCT(Vector3(1.0f, 1.0f, 0.0f),  Color(255, 255, 255, 255), Vector2(1.0f, 0.0f))
    };

    _mesh = Engine::getInstance()->getRenderer()->createMeshBuffer(indices.data(), sizeof(uint16_t), static_cast<uint32_t>(indices.size()), false,
                                                                   vertices.data(), sizeof(VertexPCT), static_cast<uint32_t>(vertices.size()), true,
                                                                   VertexPCT::ATTRIBUTES);

    _texture = Engine::getInstance()->getRenderer()->loadVideoTextureFromFile("/Users/elviss/Desktop/video/test.mov");

    return true;
}

void VideoTextureNode::update(float delta)
{

}

void VideoTextureNode::draw()
{
    Engine::getInstance()->getRenderer()->activateTexture(_texture, 0);
    Engine::getInstance()->getRenderer()->activateShader(_shader);

    Matrix4 modelViewProj = Matrix4::identity();

    _shader->setVertexShaderConstant(_uniModelViewProj, { modelViewProj });

    Engine::getInstance()->getRenderer()->drawMeshBuffer(_mesh);
}
