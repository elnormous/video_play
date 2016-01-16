//
//  VideoLayer.cpp
//  native_play
//
//  Created by Elviss Strazdins on 13/01/16.
//  Copyright © 2016 Bool Games. All rights reserved.
//

#include "VideoLayer.h"

#include <stdio.h>

extern "C"
{
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavformat/avio.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavcodec/avcodec.h>
}

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define OK 0
#define ERROR -1

#define BUFFER_SIZE 1024 * 8
#define MEMORY_STEP 1024

using namespace ouzel;

int get_frame(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx, AVFrame *pFrame, int videoStream, int64_t second)
{
    AVPacket packet;
    int      frameFinished = 0;
    int      rc;
    
    if ((pFormatCtx->duration > 0) && ((((float_t) pFormatCtx->duration / AV_TIME_BASE) - second)) < 0.1) {
        return ERROR;
    }
    
    rc = ERROR;
    // Find the nearest frame
    while (!frameFinished && av_read_frame(pFormatCtx, &packet) >= 0) {
        // Is this a packet from the video stream?
        if (packet.stream_index == videoStream) {
            // Decode video frame
            avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
            // Did we get a video frame?
            if (frameFinished) {
                rc = OK;
            }
        }
        // Free the packet that was allocated by av_read_frame
        av_packet_unref(&packet);
    }
    
    return rc;
}

float display_aspect_ratio(AVCodecContext *pCodecCtx)
{
    double aspect_ratio = av_q2d(pCodecCtx->sample_aspect_ratio);
    return ((float) pCodecCtx->width / pCodecCtx->height) * (aspect_ratio ? aspect_ratio : 1);
}


int display_width(AVCodecContext *pCodecCtx)
{
    return pCodecCtx->height * display_aspect_ratio(pCodecCtx);
}

void init()
{
    // Register all formats and codecs
    av_register_all();
    av_log_set_level(AV_LOG_ERROR);
}

int              rc, ret, videoStream;
AVFormatContext *pFormatCtx = NULL;
AVCodecContext  *pCodecCtx = NULL;
AVCodec         *pCodec = NULL;
AVFrame         *pFrame = NULL;
//size_t           uncompressed_size;
unsigned char   *bufferAVIO = NULL;
int              need_flush = 0;
char             value[10];
int              threads = 2;
int              second = 0;
AVCodecContext  *pOCodecCtx = NULL;
AVCodec         *pOCodec = NULL;
AVPacket        *packet = NULL;
AVFrame         *pFrameRGB = NULL;
struct SwsContext *scalerCtx = NULL;
AVDictionary    *input_options = NULL;
char             proto[8];

VideoLayer::VideoLayer()
{
    _shader = Engine::getInstance()->getRenderer()->getShader(SHADER_TEXTURE);
    
#ifdef OUZEL_PLATFORM_WINDOWS
    _uniModelViewProj = 0;
#else
    _uniModelViewProj = _shader->getVertexShaderConstantId("modelViewProj");
#endif
    
    std::vector<uint16_t> indices = {0, 1, 2, 1, 3, 2};
    
    Vector2 textCoords[4];
    
    textCoords[0] = Vector2(0.0f, 1.0f);
    textCoords[1] = Vector2(1.0f, 1.0f);
    textCoords[2] = Vector2(0.0f, 0.0f);
    textCoords[3] = Vector2(1.0f, 0.0f);
    
    std::vector<VertexPCT> vertices = {
        VertexPCT(Vector3(-1.0f, -1.0f, 0.0f), Color(255, 255, 255, 255), textCoords[0]),
        VertexPCT(Vector3(1.0f, -1.0f, 0.0f), Color(255, 255, 255, 255), textCoords[1]),
        VertexPCT(Vector3(-1.0f, 1.0f, 0.0f),  Color(255, 255, 255, 255), textCoords[2]),
        VertexPCT(Vector3(1.0f, 1.0f, 0.0f),  Color(255, 255, 255, 255), textCoords[3])
    };
    
    _mesh = Engine::getInstance()->getRenderer()->createMeshBuffer(indices.data(), sizeof(uint16_t), static_cast<uint32_t>(indices.size()), false,
                                                                                       vertices.data(), sizeof(VertexPCT), static_cast<uint32_t>(vertices.size()), true,
                                                                                       VertexPCT::ATTRIBUTES);
    init();
    
    
    
    
    
    rc = ERROR;
    
    bufferAVIO = (unsigned char *)malloc(BUFFER_SIZE);
    if (!bufferAVIO)
    {
        ouzel::log("Couldn't alloc AVIO buffer\n");
        return;
    }
    
    pFormatCtx = avformat_alloc_context();
    if (!pFormatCtx)
    {
        ouzel::log("Couldn't alloc AVIO buffer\n");
        return;
    }
    
    pFormatCtx->flags |= AVFMT_FLAG_NONBLOCK;
    
    if (Engine::getInstance()->getArgs().size() < 2)
    {
        return;
    }
    
    std::string stream = Engine::getInstance()->getArgs()[1];
    
    av_url_split(proto, sizeof(proto), NULL, 0,
                 NULL, 0, NULL,
                 NULL, 0, stream.c_str());
    
    if (strcmp(proto, "rtmp") == 0)
    {
        av_dict_set(&input_options, "rtmp_live", "live", 0);
    }
    
    // Open video file
    if ((ret = avformat_open_input(&pFormatCtx, stream.c_str(), NULL, &input_options)) != 0)
    {
        ouzel::log("Couldn't open file %s, error: %d\n", stream.c_str(), ret);
        return;
    }
    
    // Retrieve stream information
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        ouzel::log("Couldn't find stream information\n");
        return;
    }
    
    if ((pFormatCtx->duration > 0) && ((((float_t) pFormatCtx->duration / AV_TIME_BASE))) < 0.1) {
        ouzel::log("seconds greater than duration\n");
        rc = ERROR;
        return;
    }
    
    // Find the first video stream
    videoStream = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &pCodec, 0);
    if (videoStream == -1) {
        ouzel::log("Didn't find a video stream\n");
        return;
    }
    
    // Get a pointer to the codec context for the video stream
    pCodecCtx = pFormatCtx->streams[videoStream]->codec;
    
    AVDictionary *dict = NULL;
    sprintf(value, "%d", threads);
    av_dict_set(&dict, "threads", value, 0);
    
    // Open codec
    if ((avcodec_open2(pCodecCtx, pCodec, &dict)) < 0) {
        ouzel::log("Could not open codec\n");
        return;
    }
    
    //setup_parameters(cf, ctx, pFormatCtx, pCodecCtx);
    
    /*if (setup_filters(pFormatCtx, pCodecCtx, videoStream, &filter_graph, &buffersrc_ctx, &buffersink_ctx) < 0) {
     return;
     }*/
    
    _texture = ouzel::Engine::getInstance()->getRenderer()->createTexture(Size2(pCodecCtx->width, pCodecCtx->height), true);
}

VideoLayer::~VideoLayer()
{
    if (packet)
    {
        av_packet_free(&packet);
    }
    
    if (pOCodecCtx)
    {
        avcodec_free_context(&pOCodecCtx);
    }
    
    if (pFrameRGB) av_frame_free(&pFrameRGB);
    
    // Free the YUV frame
    if (pFrame) av_frame_free(&pFrame);
    
    if (scalerCtx) sws_freeContext(scalerCtx);
    
    // Close the codec
    if (pCodecCtx) avcodec_close(pCodecCtx);
    
    // Close the video file
    if (pFormatCtx) avformat_close_input(&pFormatCtx);
}

void VideoLayer::update(float delta)
{
    getFrame();
}

void VideoLayer::draw()
{
    Engine::getInstance()->getRenderer()->activateTexture(_texture, 0);
    Engine::getInstance()->getRenderer()->activateShader(_shader);
    
    Matrix4 modelViewProj = Matrix4::identity();
    
    _shader->setVertexShaderConstant(_uniModelViewProj, { modelViewProj });
    
    Engine::getInstance()->getRenderer()->drawMeshBuffer(_mesh);
}

int VideoLayer::getFrame()
{
    // Allocate video frame
    pFrame = av_frame_alloc();
    
    if (pFrame == NULL) {
        ouzel::log("Could not alloc frame memory\n");
        return 0xDEADBEEF;
    }
    
    if ((rc = get_frame(pFormatCtx, pCodecCtx, pFrame, videoStream, second)) == 0) {
        
        //if (pFrame->pict_type == 0)
        { // AV_PICTURE_TYPE_NONE
            need_flush = 1;
            
            /*if (filter_frame(buffersrc_ctx, buffersink_ctx, pFrame, pFrame) == AVERROR(EAGAIN)) {
             need_flush = 1;
             continue;
             }*/

            /*if (need_flush) {
             if (filter_frame(buffersrc_ctx, buffersink_ctx, NULL, pFrame) < 0) {
             return 0xDEADBEEF;
             }
             
             rc = OK;
             }*/
            
            
            /*AVFrame* newFrame = av_frame_alloc();
             int numBytes = avpicture_get_size(AV_PIX_FMT_RGB24, pFrame->width, pFrame->height);
             uint8_t *buffer= malloc(numBytes);
             
             avpicture_fill((AVPicture *)newFrame, buffer, AV_PIX_FMT_RGB24, pFrame->width, pFrame->height);
             
             img_convert((AVPicture *)newFrame, AV_PIX_FMT_RGB24,
             (AVPicture*)pFrame, pCodecCtx->pix_fmt, pCodecCtx->width,
             pCodecCtx->height);*/
            
            //uncompressed_size = pFrame->width * pFrame->height * 3;
            
            //char *newBuffer = malloc(uncompressed_size);
            //av_image_copy_to_buffer(newBuffer, uncompressed_size, pFrame->data, pFrame->linesize, AV_PIX_FMT_RGB24, pFrame->width, pFrame->height);
            
            ouzel::log("Pixel format: %d, %d", pCodecCtx->pix_fmt, AV_PIX_FMT_YUV420P);
            ouzel::log("Colorspace: %d", pFrame->colorspace);
            
            scalerCtx = sws_getContext(pCodecCtx->width,
                                       pCodecCtx->height,
                                       pCodecCtx->pix_fmt,
                                       pCodecCtx->width,
                                       pCodecCtx->height,
                                       AV_PIX_FMT_RGBA, //AV_PIX_FMT_RGB24,
                                       SWS_BILINEAR, //SWS_BICUBIC
                                       NULL, NULL, NULL);
            if (!scalerCtx)
            {
                printf("sws_getContext() failed\n");
                return 0xDEADBEEF;
            }
            
            pFrameRGB = av_frame_alloc();
            
            if (pFrameRGB == NULL)
            {
                printf("Failed to alloc frame\n");
                return 0xDEADBEEF;
            }
            
            avpicture_alloc((AVPicture *)pFrameRGB, AV_PIX_FMT_RGBA /*AV_PIX_FMT_RGB24*/, pCodecCtx->width, pCodecCtx->height);
            
            sws_scale(scalerCtx, pFrame->data, pFrame->linesize, 0, pFrame->height, pFrameRGB->data, pFrameRGB->linesize);
            
            _texture->upload(pFrameRGB->data, ouzel::Size2(pFrame->width, pFrame->height));
            
            /*pOCodec = avcodec_find_encoder (AV_CODEC_ID_PNG);
            
            if (!pOCodec)
            {
                ouzel::log("Could not find png encode");
                return 0xDEADBEEF;
            }
            
            pOCodecCtx = avcodec_alloc_context3(pOCodec);
            
            if (!pOCodecCtx) {
                ouzel::log("Failed to create codec context");
                return 0xDEADBEEF;
            }
            
            pOCodecCtx->bit_rate      = pCodecCtx->bit_rate;
            pOCodecCtx->width         = pCodecCtx->width;
            pOCodecCtx->height        = pCodecCtx->height;
            //pOCodecCtx->pix_fmt       = pCodecCtx->pix_fmt;
            pOCodecCtx->pix_fmt       = AV_PIX_FMT_RGBA; //AV_PIX_FMT_RGB24;
            //pOCodecCtx->codec_id      = AV_CODEC_ID_PNG;
            //pOCodecCtx->codec_type    = AVMEDIA_TYPE_VIDEO;
            pOCodecCtx->time_base.num = pCodecCtx->time_base.num;
            pOCodecCtx->time_base.den = pCodecCtx->time_base.den;
            
            packet = av_packet_alloc();*/
            
            
            /*int got_packet;
            
            avcodec_encode_video2(pOCodecCtx, packet, pFrameRGB, &got_packet);
            
            if (!got_packet)
            {
                ouzel::log("Didn't get packet");
                return 0xDEADBEEF;
            }
            
            //FILE* f = fopen(out_name, "wb");
            //fwrite(packet->data, packet->size, 1, f);
            //fclose(f);
            
            _texture->upload(packet->data, ouzel::Size2(pOCodecCtx->width, pOCodecCtx->height));*/
            
            rc = OK;
        }
    }
    
    if (pFrame == NULL || rc != OK) {
        ouzel::log("Failed to get frame\n");
        return 0xDEADBEEF;
    }

    
    return rc;
}