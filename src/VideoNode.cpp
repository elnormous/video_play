//
//  native_play
//

#include "VideoNode.h"

#include <cstdio>
#include <cinttypes>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace ouzel;
using namespace scene;
using namespace graphics;

VideoNode::VideoNode()
{
}

VideoNode::~VideoNode()
{
    sharedEngine->unscheduleUpdate(updateCallback);

    // Free the YUV frame
    if (frame) av_frame_free(&frame);

    if (scalerCtx) sws_freeContext(scalerCtx);

    // Close the codec
    if (codecCtx) avcodec_close(codecCtx);

    // Close the video file
    if (formatCtx) avformat_close_input(&formatCtx);
}

bool VideoNode::init()
{
    updateCallback = std::make_shared<UpdateCallback>();
    updateCallback->callback = std::bind(&VideoNode::update, this, std::placeholders::_1);

    sharedEngine->scheduleUpdate(updateCallback);

    shader = sharedEngine->getCache()->getShader(SHADER_TEXTURE);

    std::vector<uint16_t> indices = {0, 1, 2, 1, 3, 2};

    std::vector<VertexPCT> vertices = {
        VertexPCT(Vector3(-1.0f, -1.0f, 0.0f), Color(255, 255, 255, 255), Vector2(0.0f, 1.0f)),
        VertexPCT(Vector3(1.0f, -1.0f, 0.0f), Color(255, 255, 255, 255), Vector2(1.0f, 1.0f)),
        VertexPCT(Vector3(-1.0f, 1.0f, 0.0f),  Color(255, 255, 255, 255), Vector2(0.0f, 0.0f)),
        VertexPCT(Vector3(1.0f, 1.0f, 0.0f),  Color(255, 255, 255, 255), Vector2(1.0f, 0.0f))
    };

    mesh = sharedEngine->getRenderer()->createMeshBufferFromData(indices.data(), sizeof(uint16_t), static_cast<uint32_t>(indices.size()), false,
                                                                   vertices.data(), VertexPCT::ATTRIBUTES, static_cast<uint32_t>(vertices.size()), true);

    // Register all formats and codecs
    av_register_all();
    av_log_set_level(AV_LOG_ERROR);

    formatCtx = avformat_alloc_context();
    if (!formatCtx)
    {
        log("Couldn't alloc avformat context");
        return false;
    }

    formatCtx->flags |= AVFMT_FLAG_NONBLOCK;

    if (getArgs().size() < 2)
    {
        return false;
    }

    std::string stream = getArgs()[1];

    char proto[8];
    av_url_split(proto, sizeof(proto), NULL, 0,
                 NULL, 0, NULL,
                 NULL, 0, stream.c_str());

    AVDictionary* inputOptions = nullptr;

    if (strcmp(proto, "rtmp") == 0)
    {
        av_dict_set(&inputOptions, "rtmp_live", "live", 0);
    }

    // Open video file
    int ret;
    if ((ret = avformat_open_input(&formatCtx, stream.c_str(), NULL, &inputOptions)) != 0)
    {
        log("Couldn't open file %s, error: %d", stream.c_str(), ret);
        av_dict_free(&inputOptions);
        return false;
    }

    av_dict_free(&inputOptions);

    // Retrieve stream information
    if (avformat_find_stream_info(formatCtx, NULL) < 0)
    {
        log("Couldn't find stream information");
        return false;
    }

    if ((formatCtx->duration > 0) && ((((float_t)formatCtx->duration / AV_TIME_BASE))) < 0.1)
    {
        log("seconds greater than duration");
        return false;
    }

    // Find the first video stream
    videoStream = av_find_best_stream(formatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
    if (videoStream == -1)
    {
        ouzel::log("Didn't find a video stream");
        return false;
    }

    // Get a pointer to the codec context for the video stream
    codecCtx = formatCtx->streams[videoStream]->codec;

    int threads = 2;
    char value[10];
    AVDictionary *dict = NULL;
    sprintf(value, "%d", threads);
    av_dict_set(&dict, "threads", value, 0);

    // Open codec
    if ((avcodec_open2(codecCtx, codec, &dict)) < 0)
    {
        log("Could not open codec");
        return false;
    }

    texture = ouzel::sharedEngine->getRenderer()->createTexture(Size2(codecCtx->width, codecCtx->height), true, false);

    scalerCtx = sws_getContext(codecCtx->width,
                               codecCtx->height,
                               codecCtx->pix_fmt,
                               codecCtx->width,
                               codecCtx->height,
                               AV_PIX_FMT_RGBA, //AV_PIX_FMT_RGB24,
                               SWS_BILINEAR, //SWS_BICUBIC
                               NULL, NULL, NULL);
    if (!scalerCtx)
    {
        log("sws_getContext() failed");
        return false;
    }

    // Allocate video frame
    frame = av_frame_alloc();

    if (frame == NULL)
    {
        log("Failed to alloc frame");
        return false;
    }

    //setScale(Vector2(codecCtx->width / 2.0f, codecCtx->height / 2.0f));

    return true;
}

const float FPS = 25.0f;
const float FRAME_INTERVAL = 1.0f / FPS;

void VideoNode::update(float delta)
{
    if (!formatCtx)
    {
        return;
    }

    readFrame();

    sinceLastFrame += delta;

    while (sinceLastFrame >= FRAME_INTERVAL)
    {
        sinceLastFrame -= FRAME_INTERVAL;

        if (!frames.empty())
        {
            AVFrame* frame = frames.front();
            frames.pop();

            if (sinceLastFrame < FRAME_INTERVAL)
            {
                texture->upload(frame->data[0], ouzel::Size2(frame->width, frame->height));
            }

            if (frame)
            {
                avpicture_free((AVPicture*)frame);
                av_frame_free(&frame);
            }
        }
    }

    while (frames.size() > 25)
    {
        AVFrame* frame = frames.front();
        frames.pop();

        if (frame) av_frame_free(&frame);
    }
}

void VideoNode::draw(const ouzel::Matrix4& projectionMatrix, const ouzel::Matrix4& transformMatrix, const ouzel::graphics::Color& drawColor)
{
    sharedEngine->getRenderer()->activateTexture(texture, 0);
    sharedEngine->getRenderer()->activateShader(shader);

    Matrix4 modelViewProj = projectionMatrix * transformMatrix;

    float colorVector[] = { drawColor.getR(), drawColor.getG(), drawColor.getB(), drawColor.getA() };

    shader->setVertexShaderConstant(0, sizeof(Matrix4), 1, modelViewProj.m);
    shader->setPixelShaderConstant(0, sizeof(colorVector), 1, colorVector);

    sharedEngine->getRenderer()->drawMeshBuffer(mesh);
}

bool VideoNode::readFrame()
{
    bool result = false;
    AVPacket packet;

    // Find the nearest frame
    if (av_read_frame(formatCtx, &packet) >= 0)
    {
        log("Packet pts: %" PRId64, packet.pts);

        // Is this a packet from the video stream?
        if (packet.stream_index == videoStream)
        {
            int frameFinished = 0;
            // Decode video frame
            avcodec_decode_video2(codecCtx, frame, &frameFinished, &packet);
            // Did we get a video frame?
            if (frameFinished)
            {
                log("Frame decoded");

                if (frame->pts == AV_NOPTS_VALUE)
                {
                    log("No pts, pkt_pts: %" PRId64 ", pkt_dts: %" PRId64, frame->pkt_pts, frame->pkt_dts);
                }
                else
                {
                    log("pts: %" PRId64 " , pkt_pts: %" PRId64 ", pkt_dts: %" PRId64, frame->pts, frame->pkt_pts, frame->pkt_dts);
                }

                AVFrame* frameRGB = av_frame_alloc();

                if (frameRGB == NULL)
                {
                    log("Failed to alloc frame");
                }

                avpicture_alloc((AVPicture*)frameRGB, AV_PIX_FMT_RGBA /*AV_PIX_FMT_RGB24*/, codecCtx->width, codecCtx->height);

                frameRGB->width = frame->width;
                frameRGB->height = frame->height;

                sws_scale(scalerCtx, frame->data, frame->linesize, 0, frame->height, frameRGB->data, frameRGB->linesize);

                frames.push(frameRGB);

                result = true;
            }

        }
        // Free the packet that was allocated by av_read_frame
        av_packet_unref(&packet);
    }

    return result;
}
