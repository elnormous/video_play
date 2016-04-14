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
using namespace video;

VideoNode::VideoNode()
{
}

VideoNode::~VideoNode()
{
    sharedEngine->unscheduleUpdate(_updateCallback);

    // Free the YUV frame
    if (_frame) av_frame_free(&_frame);

    if (_scalerCtx) sws_freeContext(_scalerCtx);

    // Close the codec
    if (_codecCtx) avcodec_close(_codecCtx);

    // Close the video file
    if (_formatCtx) avformat_close_input(&_formatCtx);
}

bool VideoNode::init()
{
    _updateCallback = std::make_shared<UpdateCallback>();
    _updateCallback->callback = std::bind(&VideoNode::update, this, std::placeholders::_1);

    sharedEngine->scheduleUpdate(_updateCallback);

    _shader = sharedEngine->getCache()->getShader(SHADER_TEXTURE);

    std::vector<uint16_t> indices = {0, 1, 2, 1, 3, 2};

    std::vector<VertexPCT> vertices = {
        VertexPCT(Vector3(-1.0f, -1.0f, 0.0f), Color(255, 255, 255, 255), Vector2(0.0f, 1.0f)),
        VertexPCT(Vector3(1.0f, -1.0f, 0.0f), Color(255, 255, 255, 255), Vector2(1.0f, 1.0f)),
        VertexPCT(Vector3(-1.0f, 1.0f, 0.0f),  Color(255, 255, 255, 255), Vector2(0.0f, 0.0f)),
        VertexPCT(Vector3(1.0f, 1.0f, 0.0f),  Color(255, 255, 255, 255), Vector2(1.0f, 0.0f))
    };

    _mesh = sharedEngine->getRenderer()->createMeshBufferFromData(indices.data(), sizeof(uint16_t), static_cast<uint32_t>(indices.size()), false,
                                                                   vertices.data(), VertexPCT::ATTRIBUTES, static_cast<uint32_t>(vertices.size()), true);

    // Register all formats and codecs
    av_register_all();
    av_log_set_level(AV_LOG_ERROR);

    _formatCtx = avformat_alloc_context();
    if (!_formatCtx)
    {
        log("Couldn't alloc avformat context");
        return false;
    }

    _formatCtx->flags |= AVFMT_FLAG_NONBLOCK;

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
    if ((ret = avformat_open_input(&_formatCtx, stream.c_str(), NULL, &inputOptions)) != 0)
    {
        log("Couldn't open file %s, error: %d", stream.c_str(), ret);
        av_dict_free(&inputOptions);
        return false;
    }

    av_dict_free(&inputOptions);

    // Retrieve stream information
    if (avformat_find_stream_info(_formatCtx, NULL) < 0)
    {
        log("Couldn't find stream information");
        return false;
    }

    if ((_formatCtx->duration > 0) && ((((float_t)_formatCtx->duration / AV_TIME_BASE))) < 0.1)
    {
        log("seconds greater than duration");
        return false;
    }

    // Find the first video stream
    videoStream = av_find_best_stream(_formatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &_codec, 0);
    if (videoStream == -1)
    {
        ouzel::log("Didn't find a video stream");
        return false;
    }

    // Get a pointer to the codec context for the video stream
    _codecCtx = _formatCtx->streams[videoStream]->codec;

    int threads = 2;
    char value[10];
    AVDictionary *dict = NULL;
    sprintf(value, "%d", threads);
    av_dict_set(&dict, "threads", value, 0);

    // Open codec
    if ((avcodec_open2(_codecCtx, _codec, &dict)) < 0)
    {
        log("Could not open codec");
        return false;
    }

    _texture = ouzel::sharedEngine->getRenderer()->createTexture(Size2(_codecCtx->width, _codecCtx->height), true, false);

    _scalerCtx = sws_getContext(_codecCtx->width,
                                _codecCtx->height,
                                _codecCtx->pix_fmt,
                                _codecCtx->width,
                                _codecCtx->height,
                                AV_PIX_FMT_RGBA, //AV_PIX_FMT_RGB24,
                                SWS_BILINEAR, //SWS_BICUBIC
                                NULL, NULL, NULL);
    if (!_scalerCtx)
    {
        log("sws_getContext() failed");
        return false;
    }

    // Allocate video frame
    _frame = av_frame_alloc();

    if (_frame == NULL)
    {
        log("Failed to alloc frame");
        return false;
    }

    setScale(Vector2(_codecCtx->width / 2.0f, _codecCtx->height / 2.0f));

    return true;
}

const float FPS = 25.0f;
const float FRAME_INTERVAL = 1.0f / FPS;

void VideoNode::update(float delta)
{
    readFrame();

    _sinceLastFrame += delta;

    while (_sinceLastFrame >= FRAME_INTERVAL)
    {
        _sinceLastFrame -= FRAME_INTERVAL;

        if (!_frames.empty())
        {
            AVFrame* frame = _frames.front();
            _frames.pop();

            if (_sinceLastFrame < FRAME_INTERVAL)
            {
                _texture->upload(frame->data[0], ouzel::Size2(frame->width, frame->height));
            }

            if (frame)
            {
                avpicture_free((AVPicture*)frame);
                av_frame_free(&frame);
            }
        }
    }

    while (_frames.size() > 25)
    {
        AVFrame* frame = _frames.front();
        _frames.pop();

        if (frame) av_frame_free(&frame);
    }
}

void VideoNode::draw()
{
    if (LayerPtr layer = _layer.lock())
    {
        sharedEngine->getRenderer()->activateTexture(_texture, 0);
        sharedEngine->getRenderer()->activateShader(_shader);

        Matrix4 modelViewProj = layer->getCamera()->getViewProjection() * _transform;

        _shader->setVertexShaderConstant(0, { modelViewProj });

        sharedEngine->getRenderer()->drawMeshBuffer(_mesh);
    }
}

bool VideoNode::readFrame()
{
    bool result = false;
    AVPacket packet;

    // Find the nearest frame
    if (av_read_frame(_formatCtx, &packet) >= 0)
    {
        log("Packet pts: %" PRId64, packet.pts);

        // Is this a packet from the video stream?
        if (packet.stream_index == videoStream)
        {
            int frameFinished = 0;
            // Decode video frame
            avcodec_decode_video2(_codecCtx, _frame, &frameFinished, &packet);
            // Did we get a video frame?
            if (frameFinished)
            {
                log("Frame decoded");

                if (_frame->pts == AV_NOPTS_VALUE)
                {
                    log("No pts, pkt_pts: %" PRId64 ", pkt_dts: %" PRId64, _frame->pkt_pts, _frame->pkt_dts);
                }
                else
                {
                    log("pts: %" PRId64 " , pkt_pts: %" PRId64 ", pkt_dts: %" PRId64, _frame->pts, _frame->pkt_pts, _frame->pkt_dts);
                }

                AVFrame* frameRGB = av_frame_alloc();

                if (frameRGB == NULL)
                {
                    log("Failed to alloc frame");
                }

                avpicture_alloc((AVPicture*)frameRGB, AV_PIX_FMT_RGBA /*AV_PIX_FMT_RGB24*/, _codecCtx->width, _codecCtx->height);

                frameRGB->width = _frame->width;
                frameRGB->height = _frame->height;

                sws_scale(_scalerCtx, _frame->data, _frame->linesize, 0, _frame->height, frameRGB->data, frameRGB->linesize);

                _frames.push(frameRGB);

                result = true;
            }

        }
        // Free the packet that was allocated by av_read_frame
        av_packet_unref(&packet);
    }

    return result;
}
