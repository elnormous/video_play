//
//  native_play
//

#pragma once

class VideoTextureNode: public ouzel::Drawable
{
public:
    VideoTextureNode();
    virtual ~VideoTextureNode();

    virtual bool init();

    virtual void update(float delta);
    virtual void draw(const ouzel::Matrix4& projectionMatrix, const ouzel::Matrix4& transformMatrix, const ouzel::graphics::Color& drawColor) override;

protected:
    ouzel::VideoTexturePtr texture;
    ouzel::ShaderPtr shader;
    ouzel::MeshBufferPtr mesh;

    ouzel::UpdateCallbackPtr updateCallback;
};
