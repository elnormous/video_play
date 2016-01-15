//
//  VideoLayer.hpp
//  native_play
//
//  Created by Elviss Strazdins on 13/01/16.
//  Copyright © 2016 Bool Games. All rights reserved.
//

#pragma once

class VideoLayer: public ouzel::Layer
{
public:
    VideoLayer();
    virtual ~VideoLayer();
    
    virtual void update(float delta) override;
    virtual void draw() override;
    
    int getFrame();
    
protected:
    ouzel::TexturePtr _texture;
    ouzel::ShaderPtr _shader;
    ouzel::MeshBufferPtr _mesh;
    
    uint32_t _uniModelViewProj;
};
