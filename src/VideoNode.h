//
//  VideoLayer.hpp
//  native_play
//
//  Created by Elviss Strazdins on 13/01/16.
//  Copyright © 2016 Bool Games. All rights reserved.
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

class VideoNode: public ouzel::scene::Node
{
public:
    VideoNode();
    virtual ~VideoNode();
    
    virtual bool init();
    
    virtual void update(float delta);
    virtual void draw() override;
    
protected:
    bool readFrame();
    
    ouzel::video::TexturePtr _texture;
    ouzel::video::ShaderPtr _shader;
    ouzel::video::MeshBufferPtr _mesh;
    
    uint32_t _uniModelViewProj;
    
    ouzel::UpdateCallbackPtr _updateCallback;
    
    int videoStream;
    AVFormatContext* pFormatCtx = nullptr;
    AVCodecContext* pCodecCtx = nullptr;
    AVCodec* pCodec = nullptr;
    AVFrame* pFrame = nullptr;
    struct SwsContext* scalerCtx = nullptr;
    
    std::queue<AVFrame*> _frames;
    float _sinceLastFrame = 0.0f;
};
