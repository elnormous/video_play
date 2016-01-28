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

class VideoLayer: public ouzel::Layer
{
public:
    VideoLayer();
    virtual ~VideoLayer();
    
    virtual void update(float delta) override;
    virtual void draw() override;
    
    int get_frame(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx, AVFrame *pFrame, int videoStream, int64_t second);
    float display_aspect_ratio(AVCodecContext *pCodecCtx);
    int display_width(AVCodecContext *pCodecCtx);
    
    int getFrame();
    
protected:
    ouzel::TexturePtr _texture;
    ouzel::ShaderPtr _shader;
    ouzel::MeshBufferPtr _mesh;
    
    uint32_t _uniModelViewProj;
    
    int videoStream;
    AVFormatContext* pFormatCtx = nullptr;
    AVCodecContext* pCodecCtx = nullptr;
    AVCodec* pCodec = nullptr;
    AVFrame* pFrame = nullptr;
    unsigned char* bufferAVIO = nullptr;
    int need_flush = 0;
    char value[10];
    int threads = 2;
    int second = 0;
    AVCodecContext* pOCodecCtx = nullptr;
    AVCodec* pOCodec = nullptr;
    AVPacket* packet = nullptr;
    AVFrame* pFrameRGB = nullptr;
    struct SwsContext* scalerCtx = nullptr;
    AVDictionary* input_options = nullptr;
    char proto[8];
};
