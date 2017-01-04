//
//  native_play
//

#include "Video.h"

#include <cstdio>
#include <cinttypes>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace ouzel;
using namespace scene;
using namespace graphics;

Video::Video()
{
}

Video::~Video()
{
    // Free the YUV frame
    if (frame) av_frame_free(&frame);

    if (scalerCtx) sws_freeContext(scalerCtx);

    // Close the codec
    if (codecCtx) avcodec_close(codecCtx);

    // Close the video file
    if (formatCtx) avformat_close_input(&formatCtx);
}

bool Video::init(const std::string& stream)
{
    updateCallback.callback = std::bind(&Video::update, this, std::placeholders::_1);
    sharedEngine->scheduleUpdate(&updateCallback);

    shader = sharedEngine->getCache()->getShader(SHADER_TEXTURE);
    blendState = sharedEngine->getCache()->getBlendState(graphics::BLEND_ALPHA);

    std::vector<uint16_t> indices = {0, 1, 2, 1, 3, 2};

    std::vector<VertexPCT> vertices = {
        VertexPCT(Vector3(-1.0f, -1.0f, 0.0f), Color(255, 255, 255, 255), Vector2(0.0f, 1.0f)),
        VertexPCT(Vector3(1.0f, -1.0f, 0.0f), Color(255, 255, 255, 255), Vector2(1.0f, 1.0f)),
        VertexPCT(Vector3(-1.0f, 1.0f, 0.0f),  Color(255, 255, 255, 255), Vector2(0.0f, 0.0f)),
        VertexPCT(Vector3(1.0f, 1.0f, 0.0f),  Color(255, 255, 255, 255), Vector2(1.0f, 0.0f))
    };

    meshBuffer = sharedEngine->getRenderer()->createMeshBuffer();
    indexBuffer = sharedEngine->getRenderer()->createIndexBuffer();
    vertexBuffer = sharedEngine->getRenderer()->createVertexBuffer();

    indexBuffer->initFromBuffer(indices.data(), sizeof(uint16_t), static_cast<uint32_t>(indices.size()), false);
    vertexBuffer->initFromBuffer(vertices.data(), VertexPCT::ATTRIBUTES, static_cast<uint32_t>(vertices.size()), true);
    meshBuffer->init(indexBuffer, vertexBuffer);

    // Register all formats and codecs
    av_register_all();
    av_log_set_level(AV_LOG_ERROR);

    formatCtx = avformat_alloc_context();
    if (!formatCtx)
    {
        Log() << "Couldn't alloc avformat context";
        return false;
    }

    formatCtx->flags |= AVFMT_FLAG_NONBLOCK;

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
        Log() << "Couldn't open file " << stream << ", error: " << ret;
        av_dict_free(&inputOptions);
        return false;
    }

    av_dict_free(&inputOptions);

    // Retrieve stream information
    if (avformat_find_stream_info(formatCtx, NULL) < 0)
    {
        Log() << "Couldn't find stream information";
        return false;
    }

    if ((formatCtx->duration > 0) && ((((float_t)formatCtx->duration / AV_TIME_BASE))) < 0.1)
    {
        Log() << "seconds greater than duration";
        return false;
    }

    // Find the first video stream
    videoStream = av_find_best_stream(formatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
    if (videoStream == -1)
    {
        Log() << "Didn't find a video stream";
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
        Log() << "Could not open codec";
        return false;
    }

    texture = ouzel::sharedEngine->getRenderer()->createTexture();
    texture->init(Size2(codecCtx->width, codecCtx->height), true, false);

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
        Log() << "sws_getContext() failed";
        return false;
    }

    // Allocate video frame
    frame = av_frame_alloc();

    if (frame == NULL)
    {
        Log() << "Failed to alloc frame";
        return false;
    }

    //setScale(Vector2(codecCtx->width / 2.0f, codecCtx->height / 2.0f));

    return true;
}

const float FPS = 25.0f;
const float FRAME_INTERVAL = 1.0f / FPS;

void Video::update(float delta)
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
                std::vector<uint8_t> data(frame->data[0],
                                          frame->data[0] + frame->width * frame->height * 4);

                texture->setData(data, ouzel::Size2(frame->width, frame->height));
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

void Video::draw(const ouzel::Matrix4& transformMatrix,
                 const ouzel::Color& drawColor,
                 ouzel::scene::Camera* camera)
{
    Component::draw(transformMatrix, drawColor, camera);

    Matrix4 modelViewProj = camera->getRenderViewProjection() * transformMatrix;
    float colorVector[] = {drawColor.normR(), drawColor.normG(), drawColor.normB(), drawColor.normA()};

    std::vector<std::vector<float>> pixelShaderConstants(1);
    pixelShaderConstants[0] = {std::begin(colorVector), std::end(colorVector)};

    std::vector<std::vector<float>> vertexShaderConstants(1);
    vertexShaderConstants[0] = {std::begin(modelViewProj.m), std::end(modelViewProj.m)};

    sharedEngine->getRenderer()->addDrawCommand({texture},
                                                shader,
                                                pixelShaderConstants,
                                                vertexShaderConstants,
                                                blendState,
                                                meshBuffer,
                                                0,
                                                graphics::Renderer::DrawMode::TRIANGLE_LIST,
                                                0,
                                                camera->getRenderTarget(),
                                                camera->getRenderViewport(),
                                                camera->getDepthWrite(),
                                                camera->getDepthTest());
}

bool Video::readFrame()
{
    bool result = false;
    AVPacket packet;

    // Find the nearest frame
    if (av_read_frame(formatCtx, &packet) >= 0)
    {
        Log() << "Packet pts: " << packet.pts;

        // Is this a packet from the video stream?
        if (packet.stream_index == videoStream)
        {
            int frameFinished = 0;
            // Decode video frame
            avcodec_decode_video2(codecCtx, frame, &frameFinished, &packet);
            // Did we get a video frame?
            if (frameFinished)
            {
                Log() << "Frame decoded";

                if (frame->pts == AV_NOPTS_VALUE)
                {
                    Log() << "No pts, pkt_pts: " << frame->pkt_pts << ", pkt_dts: " << frame->pkt_dts;
                }
                else
                {
                    Log() << "pts: " << frame->pts << " , pkt_pts: " << frame->pkt_pts << ", pkt_dts: " << frame->pkt_dts;
                }

                AVFrame* frameRGB = av_frame_alloc();

                if (frameRGB == NULL)
                {
                    Log() << "Failed to alloc frame";
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
