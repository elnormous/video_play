//
//  VideoLayer.hpp
//  native_play
//
//  Created by Elviss Strazdins on 13/01/16.
//  Copyright © 2016 Bool Games. All rights reserved.
//

#pragma once

class VideoTextureLayer: public ouzel::Layer
{
public:
    VideoTextureLayer();
    virtual ~VideoTextureLayer();
    
    virtual bool init() override;
    
    virtual void update(float delta);
    virtual void draw() override;
    
protected:
    ouzel::VideoTexturePtr _texture;
    ouzel::ShaderPtr _shader;
    ouzel::MeshBufferPtr _mesh;
    
    uint32_t _uniModelViewProj;
    
    ouzel::UpdateCallbackPtr _updateCallback;
};
