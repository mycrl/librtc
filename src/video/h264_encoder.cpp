//
//  h264_encoder.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/3/10.
//

#include "h264_encoder.h"

void H264EncoderLayer::Release()
{
    
}

std::unique_ptr<H264Encoder> H264Encoder::Create(const webrtc::SdpVideoFormat& format)
{
    return std::make_unique<H264Encoder>(format);
}

int H264Encoder::InitEncode(const webrtc::VideoCodec* codec_settings, const Settings& settings)
{
    _codec_settings = codec_settings;
    int number_of_streams = codec_settings->numberOfSimulcastStreams;
    for (int i = 0, idx = number_of_streams - 1; i < number_of_streams; ++i, --idx)
    {
        auto encoder = _openEncoder(&codec_settings->simulcastStream[i], idx);
        if (encoder.has_value())
        {
            _encoders.push_back(encoder.value());
        }
    }
    
    return WEBRTC_VIDEO_CODEC_OK;
}

int32_t H264Encoder::RegisterEncodeCompleteCallback(webrtc::EncodedImageCallback* callback)
{
    _callback = callback;
    return WEBRTC_VIDEO_CODEC_OK;
}

int32_t H264Encoder::Encode(const webrtc::VideoFrame& frame,
                            const std::vector<webrtc::VideoFrameType>* frame_types)
{
    if (!_callback)
    {
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    
    if (!frame_types)
    {
        return WEBRTC_VIDEO_CODEC_OK;
    }
    
    auto video_frame_buf = frame.video_frame_buffer();
    auto i420_buf = video_frame_buf->GetI420();
    if (!i420_buf)
    {
        i420_buf = video_frame_buf->ToI420().get();
    }
    
    if (!i420_buf)
    {
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    
    int width = i420_buf->width();
    int height = i420_buf->height();
    const uint8_t* buf = i420_buf->DataY();
    size_t buf_size = _getI420Size(i420_buf);
    
    for (auto frame_type: *frame_types)
    {
        if (frame_type == webrtc::VideoFrameType::kEmptyFrame)
        {
            continue;
        }
        
        for (int i = 0; i < _encoders.size(); i++)
        {
            if ((_onFrame(i,
                          frame_type,
                          buf,
                          width,
                          height,
                          buf_size)) != 0)
            {
                return WEBRTC_VIDEO_CODEC_ERROR;
            }
            
            while (true)
            {
                if (_readPacket(i, frame_type, frame) != 0)
                {
                    break;
                }
            }
        }
    }
    
    return WEBRTC_VIDEO_CODEC_OK;
}

void H264Encoder::SetRates(const webrtc::VideoEncoder::RateControlParameters& parameters)
{
}

int32_t H264Encoder::Release()
{
    for (auto encoder: _encoders)
    {
        encoder.Release();
    }
    
    return WEBRTC_VIDEO_CODEC_OK;
}

int H264Encoder::_readPacket(int index,
                             webrtc::VideoFrameType frame_type,
                             const webrtc::VideoFrame& frame)
{
//    auto encoder = _encoders[index];
//    if (avcodec_receive_packet(encoder.ctx, encoder.packet) != 0)
//    {
//        return -1;
//    }
//
//    auto img_buf = webrtc::EncodedImageBuffer::Create(encoder.packet->data,
//                                                      encoder.packet->size);
//    auto qp = _h264_bitstream_parser.GetLastSliceQp();
//    if (!qp.has_value())
//    {
//        auto buf = rtc::ArrayView<const uint8_t>(encoder.packet->data,
//                                                 encoder.packet->size);
//        _h264_bitstream_parser.ParseBitstream(buf);
//    }
//
//    encoder.image.SetEncodedData(img_buf);
//    encoder.image.SetSpatialIndex(encoder.simulcast_idx);
//    encoder.image.SetTimestamp(frame.timestamp());
//    encoder.image._encodedWidth = encoder.ctx->width;
//    encoder.image._encodedHeight = encoder.ctx->height;
//    encoder.image.set_size(encoder.packet->size);
//    encoder.image._frameType = frame_type;
//    encoder.image.ntp_time_ms_ = frame.ntp_time_ms();
//    encoder.image.capture_time_ms_ = frame.render_time_ms();
//    encoder.image.rotation_ = frame.rotation();
//    encoder.image.qp_ = qp.value_or(-1);
//
//    webrtc::CodecSpecificInfo codec_specific;
//    codec_specific.codecType = webrtc::kVideoCodecH264;
//    codec_specific.codecSpecific.H264.packetization_mode = encoder.pkt_mode;
//
//    _callback->OnEncodedImage(encoder.image, &codec_specific);
//    av_packet_unref(encoder.packet);
    return 0;
}

int H264Encoder::_onFrame(int index,
                              webrtc::VideoFrameType frame_type,
                              const uint8_t* frame_buf,
                              int width,
                              int height,
                              size_t len)
{
//    int ret;
//    auto encoder = _encoders[index];
//
//    if (frame_type == webrtc::VideoFrameType::kVideoFrameKey)
//    {
//        encoder.frame->pict_type = AV_PICTURE_TYPE_I;
//    }
//    else
//    {
//        encoder.frame->pict_type = AV_PICTURE_TYPE_NONE;
//    }
//
//    ret = av_frame_make_writable(encoder.frame);
//    if (ret != 0)
//    {
//        return 0;
//    }
//
//    int need_size = av_image_fill_arrays(encoder.frame->data,
//                                         encoder.frame->linesize,
//                                         frame_buf,
//                                         (enum AVPixelFormat)encoder.frame->format,
//                                         width,
//                                         height,
//                                         1);
//    if (need_size != len) {
//        return -1;
//    }
//
//    encoder.frame->pts += 1;
//    ret = avcodec_send_frame(encoder.ctx, encoder.frame);
//    if(ret != 0)
//    {
//        return -1;
//    }
//    else
//    {
//        return 0;
//    }
}

OSStatus vt_session_set_opt(VTCompressionSessionRef session,
                        CFStringRef key,
                        uint32_t val)
{
    CFNumberRef n = CFNumberCreate(NULL, kCFNumberSInt32Type, &val);
    OSStatus code = VTSessionSetProperty(session, key, n);
    CFRelease(n);
    return code;
}

OSStatus vt_session_set_opt(VTCompressionSessionRef session,
                        CFStringRef key,
                        bool val)
{
    return VTSessionSetProperty(session, key, val ? kCFBooleanTrue : kCFBooleanFalse);
}

OSStatus vt_session_set_opt(VTCompressionSessionRef session,
                        CFStringRef key,
                        std::vector<int> val)
{
    std::vector<CFTypeRef> values;
    for (auto item: val)
    {
        values.push_back(CFNumberCreate(NULL, kCFNumberIntType, &item));
    }
    
    auto cfarray = CFArrayCreate(NULL, values.data(), val.size(), &kCFTypeArrayCallBacks);
    OSStatus code = VTSessionSetProperty(session, key, cfarray);
    
    CFRelease(cfarray);
    for (auto item: values)
    {
        CFRelease(item);
    }
    
    return code;
}

std::optional<H264EncoderLayer> H264Encoder::_openEncoder(const webrtc::SimulcastStream* stream,
                                                          int stream_idx)
{
    VTCompressionSessionRef session;
    OSStatus code = VTCompressionSessionCreate(NULL,
                               stream->width,
                               stream->height,
                               kCMVideoCodecType_H264,
                               NULL,
                               NULL, // 使用不是由 VideoToolbox 分配的像素缓冲区会增加您必须复制图像数据的机会
                               NULL,
                               <#VTCompressionOutputCallback  _Nullable outputCallback#>,
                               <#void * _Nullable outputCallbackRefCon#>,
                               session);
    if (code != 0)
    {
        return std::nullopt;
    }
    
    uint32_t key_frame_interval = _codec_settings->H264().keyFrameInterval;
    int max_bitrate = stream->maxBitrate * 1000;
    uint32_t bitrate = stream->targetBitrate * 1000;
    
    vt_session_set_opt(session, kVTCompressionPropertyKey_RealTime, true);
    vt_session_set_opt(session, kVTCompressionPropertyKey_ConstantBitRate, false);
    vt_session_set_opt(session, kVTCompressionPropertyKey_AverageBitRate, bitrate);
    vt_session_set_opt(session, kVTCompressionPropertyKey_AllowOpenGOP, key_frame_interval);
    vt_session_set_opt(session, kVTCompressionPropertyKey_MaxKeyFrameInterval, key_frame_interval);
    vt_session_set_opt(session, kVTCompressionPropertyKey_ExpectedFrameRate, (uint32_t)stream->maxFramerate);
    vt_session_set_opt(session, kVTCompressionPropertyKey_DataRateLimits, (std::vector<int>){max_bitrate});
    vt_session_set_opt(session, kVTCompressionPropertyKey_UsingHardwareAcceleratedVideoEncoder, true);
    vt_session_set_opt(session, kVTVideoEncoderSpecification_EnableHardwareAcceleratedVideoEncoder, true);
    vt_session_set_opt(session, kVTVideoEncoderSpecification_RequireHardwareAcceleratedVideoEncoder, true);
    
//    int ret;
//
//    H264EncoderLayer encoder;
//    encoder.simulcast_idx = stream_idx;
//
//    auto pars = _format.parameters;
//    auto mode = pars.find(cricket::kH264FmtpPacketizationMode);
//    if (mode != pars.end() && mode->second == "1")
//    {
//        encoder.pkt_mode = webrtc::H264PacketizationMode::NonInterleaved;
//    }
//    else
//    {
//        encoder.pkt_mode = webrtc::H264PacketizationMode::SingleNalUnit;
//    }
//
//    encoder.codec = avcodec_find_encoder_by_name("h264_videotoolbox");
//    if (!encoder.codec)
//    {
//        return std::nullopt;
//    }
//
//    encoder.ctx = avcodec_alloc_context3(encoder.codec);
//    if (encoder.ctx == NULL)
//    {
//        return std::nullopt;
//    }
//
//    encoder.ctx->width = stream->width;
//    encoder.ctx->height = stream->height;
//    encoder.ctx->pix_fmt = AV_PIX_FMT_YUV420P;
//    encoder.ctx->framerate = av_make_q(stream->maxFramerate, 1);
//    encoder.ctx->time_base = av_make_q(1, stream->maxFramerate);
//    encoder.ctx->pkt_timebase = av_make_q(1, stream->maxFramerate);
//
//    encoder.ctx->max_b_frames = 3;
//    encoder.ctx->bit_rate = stream->targetBitrate * 1000 * 0.7;
//    encoder.ctx->gop_size = _codec_settings->H264().keyFrameInterval;
//
//    ret = avcodec_open2(encoder.ctx, encoder.codec, NULL);
//    if (ret != 0)
//    {
//        return std::nullopt;
//    }
//
//    encoder.packet = av_packet_alloc();
//    if (encoder.packet == NULL)
//    {
//        return std::nullopt;
//    }
//
//    encoder.frame = av_frame_alloc();
//    if (encoder.frame == NULL)
//    {
//        return std::nullopt;
//    }
//
//    encoder.frame->format = encoder.ctx->pix_fmt;
//    encoder.frame->width = encoder.ctx->width;
//    encoder.frame->height = encoder.ctx->height;
//
//    ret = av_frame_get_buffer(encoder.frame, 32);
//    if (ret < 0)
//    {
//        return std::nullopt;
//    }
//
//    return std::move(encoder);
}

size_t H264Encoder::_getI420Size(const webrtc::I420BufferInterface* buf)
{
    size_t height = buf->height();
    size_t stride_y = buf->StrideY();
    size_t stride_u = buf->StrideU();
    size_t size_y = stride_y * height;
    size_t size_uv = stride_u * (height / 2);
    return size_y + (size_uv * 2);
}
