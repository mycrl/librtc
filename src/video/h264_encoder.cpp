//
//  h264_encoder.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/3/10.
//

#include "h264_encoder.h"
extern "C" {
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
}

constexpr webrtc::ScalabilityMode IkSupportedScalabilityModes[] = {
    webrtc::ScalabilityMode::kL1T1,
    webrtc::ScalabilityMode::kL1T2,
    webrtc::ScalabilityMode::kL1T3
};

webrtc::SdpVideoFormat ICreateH264Format(webrtc::H264Profile profile,
                                         webrtc::H264Level level,
                                         const std::string& packetization_mode,
                                         bool add_scalability_modes)
{
    const absl::optional<std::string> profile_string =
        H264ProfileLevelIdToString(webrtc::H264ProfileLevelId(profile, level));
    absl::InlinedVector<webrtc::ScalabilityMode, webrtc::kScalabilityModeCount> scalability_modes;
    
    if (add_scalability_modes)
    {
        for (const auto scalability_mode : IkSupportedScalabilityModes)
        {
            scalability_modes.push_back(scalability_mode);
        }
    }
    
    return webrtc::SdpVideoFormat(
                                  cricket::kH264CodecName,
                                  {{cricket::kH264FmtpProfileLevelId, *profile_string},
                                      {cricket::kH264FmtpLevelAsymmetryAllowed, "1"},
                                      {cricket::kH264FmtpPacketizationMode, packetization_mode}},
                                  scalability_modes);
}

std::vector<webrtc::SdpVideoFormat> ISupportedH264Codecs(bool mode /* add_scalability_modes */)
{
    return {ICreateH264Format(webrtc::H264Profile::kProfileBaseline, webrtc::H264Level::kLevel3_1, "1", mode),
        ICreateH264Format(webrtc::H264Profile::kProfileBaseline, webrtc::H264Level::kLevel3_1, "0", mode),
        ICreateH264Format(webrtc::H264Profile::kProfileConstrainedBaseline, webrtc::H264Level::kLevel3_1, "1", mode),
        ICreateH264Format(webrtc::H264Profile::kProfileConstrainedBaseline, webrtc::H264Level::kLevel3_1, "0", mode),
        ICreateH264Format(webrtc::H264Profile::kProfileMain, webrtc::H264Level::kLevel3_1, "1", mode),
        ICreateH264Format(webrtc::H264Profile::kProfileMain, webrtc::H264Level::kLevel3_1, "0", mode)};
}

std::vector<webrtc::SdpVideoFormat> H264Encoder::GetSupportedFormats()
{
    return ISupportedH264Codecs(true);
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
        auto encoder = _OpenEncoder(&codec_settings->simulcastStream[i], idx);
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
    int stride_y = i420_buf->StrideY();
    int stride_u = i420_buf->StrideU();
    int size_y = stride_y * height;
    int size_uv = stride_u * (height / 2);
    int len = size_y + (size_uv * 2);
    const uint8_t* buf = i420_buf->DataY();
    
    for (auto frame_type: *frame_types)
    {
        if (frame_type == webrtc::VideoFrameType::kEmptyFrame)
        {
            continue;
        }
        
        for (int i = 0; i < _encoders.size(); i++)
        {
            if ((_OnFrame(i,
                          frame_type,
                          buf,
                          width,
                          height,
                          len)) != 0)
            {
                break;
            }
            
            while (true)
            {
                if (_ReadPacket(i, frame_type, frame) != 0)
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
    for (auto encoder: _encoders)
    {
        encoder.ctx->bit_rate = parameters.bitrate.get_sum_bps() / 1000;
        encoder.ctx->framerate = av_make_q(parameters.framerate_fps, 1);
        encoder.ctx->time_base = av_make_q(1, parameters.framerate_fps);
        encoder.ctx->pkt_timebase = av_make_q(1, parameters.framerate_fps);
    }
}

int32_t H264Encoder::Release()
{
    for (auto encoder: _encoders)
    {
        encoder.Release();
    }
    
    return WEBRTC_VIDEO_CODEC_OK;
}

int H264Encoder::_ReadPacket(int index,
                             webrtc::VideoFrameType frame_type,
                             const webrtc::VideoFrame& frame)
{
    auto encoder = _encoders[index];
    if (avcodec_receive_packet(encoder.ctx, encoder.packet) != 0)
    {
        return -1;
    }
    
    auto img_buf = webrtc::EncodedImageBuffer::Create(encoder.packet->data,
                                                      encoder.packet->size);
    auto qp = _h264_bitstream_parser.GetLastSliceQp();
    if (!qp.has_value())
    {
        auto buf = rtc::ArrayView<const uint8_t>(encoder.packet->data,
                                                 encoder.packet->size);
        _h264_bitstream_parser.ParseBitstream(buf);
    }
    
    encoder.image.SetEncodedData(img_buf);
    encoder.image.SetSpatialIndex(encoder.simulcast_idx);
    encoder.image.SetTimestamp(frame.timestamp());
    encoder.image._encodedWidth = encoder.ctx->width;
    encoder.image._encodedHeight = encoder.ctx->height;
    encoder.image.set_size(encoder.packet->size);
    encoder.image._frameType = frame_type;
    encoder.image.ntp_time_ms_ = frame.ntp_time_ms();
    encoder.image.capture_time_ms_ = frame.render_time_ms();
    encoder.image.rotation_ = frame.rotation();
    encoder.image.qp_ = qp.value_or(-1);
    
    webrtc::CodecSpecificInfo codec_specific;
    codec_specific.codecType = webrtc::kVideoCodecH264;
    codec_specific.codecSpecific.H264.packetization_mode = encoder.pkt_mode;
    
    _callback->OnEncodedImage(encoder.image, &codec_specific);
    av_packet_unref(encoder.packet);
    return 0;
}

int H264Encoder::_OnFrame(int index,
                          webrtc::VideoFrameType frame_type,
                          const uint8_t* frame_buf,
                          int width,
                          int height,
                          size_t len)
{
    int ret;
    auto encoder = _encoders[index];
    
    if (frame_type == webrtc::VideoFrameType::kVideoFrameKey)
    {
        encoder.frame->pict_type = AV_PICTURE_TYPE_I;
    }
    else
    {
        encoder.frame->pict_type = AV_PICTURE_TYPE_NONE;
    }
    
    ret = av_frame_make_writable(encoder.frame);
    if (ret != 0)
    {
        return 0;
    }
    
    int need_size = av_image_fill_arrays(encoder.frame->data,
                                         encoder.frame->linesize,
                                         frame_buf,
                                         (enum AVPixelFormat)encoder.frame->format,
                                         width,
                                         height,
                                         1);
    if (need_size != len) {
        return -1;
    }
    
    ret = avcodec_send_frame(encoder.ctx, encoder.frame);
    if(ret != 0)
    {
        return -1;
    }
    
    encoder.frame->pts ++;
    encoder.frame->pkt_dts ++;
    return 0;
}

std::optional<H264EncoderLayer> H264Encoder::_OpenEncoder(const webrtc::SimulcastStream* stream,
                                                          int stream_idx)
{
    int ret;
    
    H264EncoderLayer encoder;
    encoder.simulcast_idx = stream_idx;
    
    auto pars = _format.parameters;
    auto mode = pars.find(cricket::kH264FmtpPacketizationMode);
    if (mode != pars.end() && mode->second == "1")
    {
        encoder.pkt_mode = webrtc::H264PacketizationMode::NonInterleaved;
    }
    else
    {
        encoder.pkt_mode = webrtc::H264PacketizationMode::SingleNalUnit;
    }

    for (std::string name: {"h264_videotoolbox",
                            "libx264"})
    {
        encoder.codec = avcodec_find_encoder_by_name(name.c_str());
        if (encoder.codec)
        {
            encoder.name = name;
            break;
        }
    }

    if (!encoder.codec)
    {
        return std::nullopt;
    }
    
    encoder.ctx = avcodec_alloc_context3(encoder.codec);
    if (encoder.ctx == NULL)
    {
        return std::nullopt;
    }
    
    encoder.ctx->max_b_frames = 0;
    encoder.ctx->width = stream->width;
    encoder.ctx->height = stream->height;
    encoder.ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    encoder.ctx->bit_rate = stream->targetBitrate * 1000 * 0.7;
    encoder.ctx->framerate = av_make_q(stream->maxFramerate, 1);
    encoder.ctx->time_base = av_make_q(1, stream->maxFramerate);
    encoder.ctx->pkt_timebase = av_make_q(1, stream->maxFramerate);
    encoder.ctx->gop_size = _codec_settings->H264().keyFrameInterval;
    
    if (encoder.name == "h264_videotoolbox")
    {
        av_opt_set_int(encoder.ctx->priv_data, "prio_speed", 1, 0);
        av_opt_set_int(encoder.ctx->priv_data, "realtime", 0, 0);
    }
    else
    {
        av_opt_set(encoder.ctx->priv_data, "tune", "zerolatency", 0);
    }
    
    ret = avcodec_open2(encoder.ctx, encoder.codec, NULL);
    if (ret != 0)
    {
        return std::nullopt;
    }
    
    encoder.packet = av_packet_alloc();
    if (encoder.packet == NULL)
    {
        return std::nullopt;
    }
    
    encoder.frame = av_frame_alloc();
    if (encoder.frame == NULL)
    {
        return std::nullopt;
    }
    
    encoder.frame->format = encoder.ctx->pix_fmt;
    encoder.frame->width = encoder.ctx->width;
    encoder.frame->height = encoder.ctx->height;
    ret = av_frame_get_buffer(encoder.frame, 32);
    if (ret < 0)
    {
        return std::nullopt;
    }
    
    return std::move(encoder);
}
