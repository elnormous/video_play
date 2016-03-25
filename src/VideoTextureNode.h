//
//  native_play
//

#pragma once

class VideoTextureNode: public ouzel::Node
{
public:
    VideoTextureNode();
    virtual ~VideoTextureNode();

    virtual bool init();

    virtual void update(float delta);
    virtual void draw() override;

protected:
    ouzel::VideoTexturePtr _texture;
    ouzel::ShaderPtr _shader;
    ouzel::MeshBufferPtr _mesh;

    uint32_t _uniModelViewProj;

    ouzel::UpdateCallbackPtr _updateCallback;
};
