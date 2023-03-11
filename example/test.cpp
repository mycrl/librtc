//
//  test.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/2/26.
//

#include <stdio.h>

FFmpegH264EncoderImpl(const cricket::VideoCodec &codec, bool hardware_accelerate);
   ~FFmpegH264EncoderImpl() override;

   // |max_payload_size| is ignored.
   // The following members of |codec_settings| are used. The rest are ignored.
   // - codecType (must be kVideoCodecH264)
   // - targetBitrate
   // - maxFramerate
   // - width
   // - height
   // 初始化编码器
   int32_t InitEncode(const webrtc::VideoCodec *codec_settings,
                      int32_t number_of_cores,
                      size_t max_payload_size) override;

   // 释放资源
   int32_t Release() override;

   // 当我们编码完成时，通过该回调上交视频帧
   int32_t RegisterEncodeCompleteCallback(
           webrtc::EncodedImageCallback *callback) override;

   // WebRTC自己的码率控制器，它会根据当前网络情况，修改码率
   int32_t SetRateAllocation(const webrtc::VideoBitrateAllocation &bitrate_allocation,
                             uint32_t framerate) override;

   // The result of encoding - an EncodedImage and RTPFragmentationHeader - are
   // passed to the encode complete callback.
   int32_t Encode(const webrtc::VideoFrame &frame,
                  const webrtc::CodecSpecificInfo *codec_specific_info,
                  const std::vector<webrtc::FrameType> *frame_types) override;

// 在实现这个接口时，参考了 WebRTC 官方的 OpenH264Encoder，需要注意的是 WebRTC 是能支持 Simulcast 的，所以这个的编码实例可能会有多个，也就是说一个 Stream 对应一个编码实例。接下来，我讲逐步讲解我的实现方案，因为这个地方比较复杂。先介绍一下我这里定义的结构体和成员变量吧：

// 用该结构体保存一个编码实例的所有相关资源
typedef struct {
   AVCodec *codec = nullptr;        //指向编解码器实例
   AVFrame *frame = nullptr;        //保存解码之后/编码之前的像素数据
   AVCodecContext *context = nullptr;    //编解码器上下文，保存编解码器的一些参数设置
   AVPacket *pkt = nullptr;        //码流包结构，包含编码码流数据
} CodecCtx;

// 编码器实例
std::vector<CodecCtx *> encoders_;

// 编码器参数
std::vector<LayerConfig> configurations_;

// 编码完成后的图片
std::vector<webrtc::EncodedImage> encoded_images_;

// 图片缓存部分
std::vector<std::unique_ptr<uint8_t[]>> encoded_image_buffers_;

// 编码相关配置
webrtc::VideoCodec codec_;
webrtc::H264PacketizationMode packetization_mode_;
size_t max_payload_size_;
int32_t number_of_cores_;

// 编码完成后的回调
webrtc::EncodedImageCallback *encoded_image_callback_;

FFmpegH264EncoderImpl::FFmpegH264EncoderImpl(const cricket::VideoCodec &codec, bool hardware)
           : packetization_mode_(webrtc::H264PacketizationMode::SingleNalUnit),
             max_payload_size_(0),
             hardware_accelerate(hardware),
             number_of_cores_(0),
             encoded_image_callback_(nullptr),
             has_reported_init_(false),
             has_reported_error_(false) 
{
   RTC_CHECK(cricket::CodecNamesEq(codec.name, cricket::kH264CodecName));
   std::string packetization_mode_string;
   if (codec.GetParam(cricket::kH264FmtpPacketizationMode,
                      &packetization_mode_string) &&
       packetization_mode_string == "1") {
       packetization_mode_ = webrtc::H264PacketizationMode::NonInterleaved;
   }

   encoded_images_.reserve(webrtc::kMaxSimulcastStreams);
   encoded_image_buffers_.reserve(webrtc::kMaxSimulcastStreams);
   encoders_.reserve(webrtc::kMaxSimulcastStreams);
   configurations_.reserve(webrtc::kMaxSimulcastStreams);

}

int32_t FFmpegH264EncoderImpl::InitEncode(const webrtc::VideoCodec *inst,
                                             int32_t number_of_cores,
                                             size_t max_payload_size) {

       if (!inst || inst->codecType != webrtc::kVideoCodecH264) {
           ReportError();
           return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;

       }

       if (inst->maxFramerate == 0) {
           ReportError();
           return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;

       }

       if (inst->width < 1 || inst->height < 1) {
           ReportError();
           return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;

       }

       int32_t release_ret = Release();
       if (release_ret != WEBRTC_VIDEO_CODEC_OK) {
           ReportError();
           return release_ret;

       }

       int number_of_streams = webrtc::SimulcastUtility::NumberOfSimulcastStreams(*inst);
       bool doing_simulcast = (number_of_streams > 1);

       if (doing_simulcast && (!webrtc::SimulcastUtility::ValidSimulcastResolutions(
               *inst, number_of_streams) ||
                               !webrtc::SimulcastUtility::ValidSimulcastTemporalLayers(
                                       *inst, number_of_streams))) {
           return WEBRTC_VIDEO_CODEC_ERR_SIMULCAST_PARAMETERS_NOT_SUPPORTED;

       }

       encoded_images_.resize(static_cast<unsigned long>(number_of_streams));
       encoded_image_buffers_.resize(static_cast<unsigned long>(number_of_streams));
       encoders_.resize(static_cast<unsigned long>(number_of_streams));
       configurations_.resize(static_cast<unsigned long>(number_of_streams));

       for (int i = 0; i < number_of_streams; i++) {
           encoders_[i] = new CodecCtx();

       }

       number_of_cores_ = number_of_cores;
       max_payload_size_ = max_payload_size;
       codec_ = *inst;

       // Code expects simulcastStream resolutions to be correct, make sure they are
       // filled even when there are no simulcast layers.
       if (codec_.numberOfSimulcastStreams == 0) {
           codec_.simulcastStream[0].width = codec_.width;
           codec_.simulcastStream[0].height = codec_.height;
       }

       for (int i = 0, idx = number_of_streams - 1; i < number_of_streams;
            ++i, --idx) {
           // Temporal layers still not supported.
           if (inst->simulcastStream[i].numberOfTemporalLayers > 1) {
               Release();
               return WEBRTC_VIDEO_CODEC_ERR_SIMULCAST_PARAMETERS_NOT_SUPPORTED;
           }

           // Set internal settings from codec_settings
           configurations_[i].simulcast_idx = idx;
           configurations_[i].sending = false;
           configurations_[i].width = codec_.simulcastStream[idx].width;
           configurations_[i].height = codec_.simulcastStream[idx].height;
           configurations_[i].max_frame_rate = static_cast<float>(codec_.maxFramerate);
           configurations_[i].frame_dropping_on = codec_.H264()->frameDroppingOn;
           configurations_[i].key_frame_interval = codec_.H264()->keyFrameInterval;

           // Codec_settings uses kbits/second; encoder uses bits/second.
           configurations_[i].max_bps = codec_.maxBitrate * 1000;
           configurations_[i].target_bps = codec_.startBitrate * 1000;
           if (!OpenEncoder(encoders_[i], configurations_[i])) {
               Release();
               ReportError();
               return WEBRTC_VIDEO_CODEC_ERROR;

           }

           // Initialize encoded image. Default buffer size: size of unencoded data.
           encoded_images_[i]._size =
                   CalcBufferSize(webrtc::VideoType::kI420, codec_.simulcastStream[idx].width,
                                  codec_.simulcastStream[idx].height);
           encoded_images_[i]._buffer = new uint8_t[encoded_images_[i]._size];
           encoded_image_buffers_[i].reset(encoded_images_[i]._buffer);
           encoded_images_[i]._completeFrame = true;
           encoded_images_[i]._encodedWidth = codec_.simulcastStream[idx].width;
           encoded_images_[i]._encodedHeight = codec_.simulcastStream[idx].height;
           encoded_images_[i]._length = 0;
       }

       webrtc::SimulcastRateAllocator init_allocator(codec_);
       webrtc::BitrateAllocation allocation = init_allocator.GetAllocation(
               codec_.startBitrate * 1000, codec_.maxFramerate);
       return SetRateAllocation(allocation, codec_.maxFramerate);
   }



   // OpenEncoder函数是创建编码器的过程，这个函数中有一个隐晦的点是创建AVFrame时一定要记得设置为32内存对齐，这个之前我们在采集图像数据的时候提过
   bool FFmpegH264EncoderImpl::OpenEncoder(FFmpegH264EncoderImpl::CodecCtx *ctx, H264Encoder::LayerConfig &config) {
       int ret;

       /* find the mpeg1 video encoder */
   #ifdef WEBRTC_LINUX
       if (hardware_accelerate) {
           ctx->codec = avcodec_find_encoder_by_name("h264_nvenc");
       }
   #endif

       if (!ctx->codec) {
           ctx->codec = avcodec_find_encoder_by_name("libx264");
       }

       if (!ctx->codec) {
           WEBRTC_LOG("Codec not found", ERROR);
           return false;

       }

       WEBRTC_LOG("Open encoder: " + std::string(ctx->codec->name) + ", and generate frame, packet", INFO);

       ctx->context = avcodec_alloc_context3(ctx->codec);
       if (!ctx->context) {
           WEBRTC_LOG("Could not allocate video codec context", ERROR);
           return false;

       }
       
       config.target_bps = config.max_bps;
       SetContext(ctx, config, true);

       /* open it */
       ret = avcodec_open2(ctx->context, ctx->codec, nullptr);
       if (ret < 0) {
           WEBRTC_LOG("Could not open codec, error code:" + std::to_string(ret), ERROR);
           avcodec_free_context(&(ctx->context));
           return false;

       }

       ctx->frame = av_frame_alloc();
       if (!ctx->frame) {
           WEBRTC_LOG("Could not allocate video frame", ERROR);
           return false;

       }

       ctx->frame->format = ctx->context->pix_fmt;
       ctx->frame->width = ctx->context->width;
       ctx->frame->height = ctx->context->height;
       ctx->frame->color_range = ctx->context->color_range;
       
       /* the image can be allocated by any means and av_image_alloc() is
        * just the most convenient way if av_malloc() is to be used */
       ret = av_image_alloc(ctx->frame->data, ctx->frame->linesize, ctx->context->width, ctx->context->height,
                            ctx->context->pix_fmt, 32);

       if (ret < 0) {
           WEBRTC_LOG("Could not allocate raw picture buffer", ERROR);
           return false;

       }
       
       ctx->frame->pts = 1;
       ctx->pkt = av_packet_alloc();
       return true;

   }



   // 设置FFmpeg编码器的参数

   void FFmpegH264EncoderImpl::SetContext(CodecCtx *ctx, H264Encoder::LayerConfig &config, bool init) {

       if (init) {

           AVRational rational = {1, 25};

           ctx->context->time_base = rational;

           ctx->context->max_b_frames = 0;

           ctx->context->pix_fmt = AV_PIX_FMT_YUV420P;

           ctx->context->codec_type = AVMEDIA_TYPE_VIDEO;

           ctx->context->codec_id = AV_CODEC_ID_H264;

           ctx->context->gop_size = config.key_frame_interval;

           ctx->context->color_range = AVCOL_RANGE_JPEG;

           // 设置两个参数让编码过程更快

           if (std::string(ctx->codec->name) == "libx264") {

               av_opt_set(ctx->context->priv_data, "preset", "ultrafast", 0);

               av_opt_set(ctx->context->priv_data, "tune", "zerolatency", 0);

           }

           av_log_set_level(AV_LOG_ERROR);

           WEBRTC_LOG("Init bitrate: " + std::to_string(config.target_bps), INFO);

       } else {

           WEBRTC_LOG("Change bitrate: " + std::to_string(config.target_bps), INFO);

       }

       config.key_frame_request = true;

       ctx->context->width = config.width;

       ctx->context->height = config.height;



       ctx->context->bit_rate = config.target_bps * 0.7;

       ctx->context->rc_max_rate = config.target_bps * 0.85;

       ctx->context->rc_min_rate = config.target_bps * 0.1;

       ctx->context->rc_buffer_size = config.target_bps * 2; // buffer_size变化，触发libx264的码率编码，如果不设置这个前几条不生效

   #ifdef WEBRTC_LINUX

       if (std::string(ctx->codec->name) == "h264_nvenc") { // 使用类似于Java反射的思想，设置h264_nvenc的码率

           NvencContext* nvenc_ctx = (NvencContext*)ctx->context->priv_data;

           nvenc_ctx->encode_config.rcParams.averageBitRate = ctx->context->bit_rate;

           nvenc_ctx->encode_config.rcParams.maxBitRate = ctx->context->rc_max_rate;

           return;

       }

   #endif

   }
int32_t FFmpegH264EncoderImpl::RegisterEncodeCompleteCallback(

           webrtc::EncodedImageCallback *callback) {

       encoded_image_callback_ = callback;

       return WEBRTC_VIDEO_CODEC_OK;

   }



   int32_t FFmpegH264EncoderImpl::SetRateAllocation(

           const webrtc::BitrateAllocation &bitrate,

           uint32_t new_framerate) {

       if (encoders_.empty())

           return WEBRTC_VIDEO_CODEC_UNINITIALIZED;



       if (new_framerate < 1)

           return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;



       if (bitrate.get_sum_bps() == 0) {

           // Encoder paused, turn off all encoding.

           for (auto &configuration : configurations_)

               configuration.SetStreamState(false);

           return WEBRTC_VIDEO_CODEC_OK;

       }



       // At this point, bitrate allocation should already match codec settings.

       if (codec_.maxBitrate > 0)

           RTC_DCHECK_LE(bitrate.get_sum_kbps(), codec_.maxBitrate);

       RTC_DCHECK_GE(bitrate.get_sum_kbps(), codec_.minBitrate);

       if (codec_.numberOfSimulcastStreams > 0)

           RTC_DCHECK_GE(bitrate.get_sum_kbps(), codec_.simulcastStream[0].minBitrate);



       codec_.maxFramerate = new_framerate;



       size_t stream_idx = encoders_.size() - 1;

       for (size_t i = 0; i < encoders_.size(); ++i, --stream_idx) {

           // Update layer config.

           configurations_[i].target_bps = bitrate.GetSpatialLayerSum(stream_idx);

           configurations_[i].max_frame_rate = static_cast<float>(new_framerate);



           if (configurations_[i].target_bps) {

               configurations_[i].SetStreamState(true);

               SetContext(encoders_[i], configurations_[i], false);

           } else {

               configurations_[i].SetStreamState(false);

           }

       }



       return WEBRTC_VIDEO_CODEC_OK;

   }

// 放松完了，让我们来看看最后一块难啃的骨头吧，没错，就是编码过程了，这块看似简单实则有个大坑。

   int32_t FFmpegH264EncoderImpl::Encode(const webrtc::VideoFrame &input_frame,

                                         const webrtc::CodecSpecificInfo *codec_specific_info,

                                         const std::vector<webrtc::FrameType> *frame_types) {

       // 先进行一些常规检查

       if (encoders_.empty()) {

           ReportError();

           return WEBRTC_VIDEO_CODEC_UNINITIALIZED;

       }

       if (!encoded_image_callback_) {

           RTC_LOG(LS_WARNING)

               << "InitEncode() has been called, but a callback function "

               << "has not been set with RegisterEncodeCompleteCallback()";

           ReportError();

           return WEBRTC_VIDEO_CODEC_UNINITIALIZED;

       }



       // 获取视频帧

       webrtc::I420BufferInterface *frame_buffer = (webrtc::I420BufferInterface *) input_frame.video_frame_buffer().get();

       // 检查下一帧是否需要关键帧，一般进行码率变化时，会设定下一帧发送关键帧

       bool send_key_frame = false;

       for (auto &configuration : configurations_) {

           if (configuration.key_frame_request && configuration.sending) {

               send_key_frame = true;

               break;

           }

       }

       if (!send_key_frame && frame_types) {

           for (size_t i = 0; i < frame_types->size() && i < configurations_.size();

                ++i) {

               if ((*frame_types)[i] == webrtc::kVideoFrameKey && configurations_[i].sending) {

                   send_key_frame = true;

                   break;

               }

           }

       }



       RTC_DCHECK_EQ(configurations_[0].width, frame_buffer->width());

       RTC_DCHECK_EQ(configurations_[0].height, frame_buffer->height());



       // Encode image for each layer.

       for (size_t i = 0; i < encoders_.size(); ++i) {

           // EncodeFrame input.

           copyFrame(encoders_[i]->frame, frame_buffer);

           if (!configurations_[i].sending) {

               continue;

           }

           if (frame_types != nullptr) {

               // Skip frame?

               if ((*frame_types)[i] == webrtc::kEmptyFrame) {

                   continue;

               }

           }

           // 控制编码器发送关键帧

           if (send_key_frame || encoders_[i]->frame->pts % configurations_[i].key_frame_interval == 0) {

               // API doc says ForceIntraFrame(false) does nothing, but calling this

               // function forces a key frame regardless of the |bIDR| argument's value.

               // (If every frame is a key frame we get lag/delays.)

               encoders_[i]->frame->key_frame = 1;

               encoders_[i]->frame->pict_type = AV_PICTURE_TYPE_I;

               configurations_[i].key_frame_request = false;

           } else {

               encoders_[i]->frame->key_frame = 0;

               encoders_[i]->frame->pict_type = AV_PICTURE_TYPE_P;

           }



           // Encode!编码过程

           int got_output;

           int enc_ret;

           // 给编码器喂图片

           enc_ret = avcodec_send_frame(encoders_[i]->context, encoders_[i]->frame);

           if (enc_ret != 0) {

               WEBRTC_LOG("FFMPEG send frame failed, returned " + std::to_string(enc_ret), ERROR);

               ReportError();

               return WEBRTC_VIDEO_CODEC_ERROR;

           }

           encoders_[i]->frame->pts++;

           while (enc_ret >= 0) {

               // 从编码器接受视频帧

               enc_ret = avcodec_receive_packet(encoders_[i]->context, encoders_[i]->pkt);

               if (enc_ret == AVERROR(EAGAIN) || enc_ret == AVERROR_EOF) {

                   break;

               } else if (enc_ret < 0) {

                   WEBRTC_LOG("FFMPEG receive frame failed, returned " + std::to_string(enc_ret), ERROR);

                   ReportError();

                   return WEBRTC_VIDEO_CODEC_ERROR;

               }



               // 将编码器返回的帧转化为WebRTC需要的帧类型

               encoded_images_[i]._encodedWidth = static_cast<uint32_t>(configurations_[i].width);

               encoded_images_[i]._encodedHeight = static_cast<uint32_t>(configurations_[i].height);

               encoded_images_[i].SetTimestamp(input_frame.timestamp());

               encoded_images_[i].ntp_time_ms_ = input_frame.ntp_time_ms();

               encoded_images_[i].capture_time_ms_ = input_frame.render_time_ms();

               encoded_images_[i].rotation_ = input_frame.rotation();

               encoded_images_[i].content_type_ =

                       (codec_.mode == webrtc::VideoCodecMode::kScreensharing)

                       ? webrtc::VideoContentType::SCREENSHARE

                       : webrtc::VideoContentType::UNSPECIFIED;

               encoded_images_[i].timing_.flags = webrtc::VideoSendTiming::kInvalid;

               encoded_images_[i]._frameType = ConvertToVideoFrameType(encoders_[i]->frame);



               // Split encoded image up into fragments. This also updates

               // |encoded_image_|.

               // 这里就是前面提到的大坑，FFmpeg编码出来的视频帧每个NALU之间可能以0001作为头，也会出现以001作为头的情况

               // 而WebRTC只识别以0001作为头的NALU

               // 所以我接下来要处理一下编码器输出的视频帧，并生成一个RTC报文的头部来描述该帧的数据

               webrtc::RTPFragmentationHeader frag_header;

               RtpFragmentize(&encoded_images_[i], &encoded_image_buffers_[i], *frame_buffer, encoders_[i]->pkt,

                              &frag_header);

               av_packet_unref(encoders_[i]->pkt);

               // Encoder can skip frames to save bandwidth in which case

               // |encoded_images_[i]._length| == 0.

               if (encoded_images_[i]._length > 0) {

                   // Parse QP.

                   h264_bitstream_parser_.ParseBitstream(encoded_images_[i]._buffer,

                                                         encoded_images_[i]._length);

                   h264_bitstream_parser_.GetLastSliceQp(&encoded_images_[i].qp_);



                   // Deliver encoded image.

                   webrtc::CodecSpecificInfo codec_specific;

                   codec_specific.codecType = webrtc::kVideoCodecH264;

                   codec_specific.codecSpecific.H264.packetization_mode =

                           packetization_mode_;

                   codec_specific.codecSpecific.H264.simulcast_idx = static_cast<uint8_t>(configurations_[i].simulcast_idx);

                   encoded_image_callback_->OnEncodedImage(encoded_images_[i],

                                                           &codec_specific, &frag_header);

               }

           }

       }



       return WEBRTC_VIDEO_CODEC_OK;

   }

下面就是进行 NAL 转换以及提取 RTP 头部信息的过程：

   // Helper method used by FFmpegH264EncoderImpl::Encode.

   // Copies the encoded bytes from |info| to |encoded_image| and updates the

   // fragmentation information of |frag_header|. The |encoded_image->_buffer| may

   // be deleted and reallocated if a bigger buffer is required.

   //

   // After OpenH264 encoding, the encoded bytes are stored in |info| spread out

   // over a number of layers and "NAL units". Each NAL unit is a fragment starting

   // with the four-byte start code {0,0,0,1}. All of this data (including the

   // start codes) is copied to the |encoded_image->_buffer| and the |frag_header|

   // is updated to point to each fragment, with offsets and lengths set as to

   // exclude the start codes.

   void FFmpegH264EncoderImpl::RtpFragmentize(webrtc::EncodedImage *encoded_image,

                                              std::unique_ptr<uint8_t[]> *encoded_image_buffer,

                                              const webrtc::VideoFrameBuffer &frame_buffer, AVPacket *packet,

                                              webrtc::RTPFragmentationHeader *frag_header) {

       std::list<int> data_start_index;

       std::list<int> data_length;

       int payload_length = 0;

       // 以001 或者 0001 作为开头的情况下，遍历出所有的NAL并记录NALU数据开始的下标和NALU数据长度

       for (int i = 2; i < packet->size; i++) {

           if (i > 2

               && packet->data[i - 3] == start_code[0]

               && packet->data[i - 2] == start_code[1]

               && packet->data[i - 1] == start_code[2]

               && packet->data[i] == start_code[3]) {

               if (!data_start_index.empty()) {

                   data_length.push_back((i - 3 - data_start_index.back()));

               }

               data_start_index.push_back(i + 1);

           } else if (packet->data[i - 2] == start_code[1] &&

                      packet->data[i - 1] == start_code[2] &&

                      packet->data[i] == start_code[3]) {

               if (!data_start_index.empty()) {

                   data_length.push_back((i - 2 - data_start_index.back()));

               }

               data_start_index.push_back(i + 1);

           }

       }

       if (!data_start_index.empty()) {

           data_length.push_back((packet->size - data_start_index.back()));

       }



       for (auto &it : data_length) {

           payload_length += +it;

       }

       // Calculate minimum buffer size required to hold encoded data.

       auto required_size = payload_length + data_start_index.size() * 4;

       if (encoded_image->_size < required_size) {

           // Increase buffer size. Allocate enough to hold an unencoded image, this

           // should be more than enough to hold any encoded data of future frames of

           // the same size (avoiding possible future reallocation due to variations in

           // required size).

           encoded_image->_size = CalcBufferSize(

                   webrtc::VideoType::kI420, frame_buffer.width(), frame_buffer.height());

           if (encoded_image->_size < required_size) {

               // Encoded data > unencoded data. Allocate required bytes.

               WEBRTC_LOG("Encoding produced more bytes than the original image data! Original bytes: " +

                          std::to_string(encoded_image->_size) + ", encoded bytes: " + std::to_string(required_size) + ".",

                          WARNING);

               encoded_image->_size = required_size;

           }

           encoded_image->_buffer = new uint8_t[encoded_image->_size];

           encoded_image_buffer->reset(encoded_image->_buffer);

       }

       // Iterate layers and NAL units, note each NAL unit as a fragment and copy

       // the data to |encoded_image->_buffer|.

       int index = 0;

       encoded_image->_length = 0;

       frag_header->VerifyAndAllocateFragmentationHeader(data_start_index.size());

       for (auto it_start = data_start_index.begin(), it_length = data_length.begin();

            it_start != data_start_index.end(); ++it_start, ++it_length, ++index) {

           memcpy(encoded_image->_buffer + encoded_image->_length, start_code, sizeof(start_code));

           encoded_image->_length += sizeof(start_code);

           frag_header->fragmentationOffset[index] = encoded_image->_length;

           memcpy(encoded_image->_buffer + encoded_image->_length, packet->data + *it_start,

                  static_cast<size_t>(*it_length));

           encoded_image->_length += *it_length;

           frag_header->fragmentationLength[index] = static_cast<size_t>(*it_length);

       }

   }

最后，是非常简单的编码器释放的过程：

   int32_t FFmpegH264EncoderImpl::Release() {

       while (!encoders_.empty()) {

           CodecCtx *encoder = encoders_.back();

           CloseEncoder(encoder);

           encoders_.pop_back();

       }

       configurations_.clear();

       encoded_images_.clear();

       encoded_image_buffers_.clear();

       return WEBRTC_VIDEO_CODEC_OK;

   }



   void FFmpegH264EncoderImpl::CloseEncoder(FFmpegH264EncoderImpl::CodecCtx *ctx) {

       if (ctx) {

           if (ctx->context) {

               avcodec_close(ctx->context);

               avcodec_free_context(&(ctx->context));

           }

           if (ctx->frame) {

               av_frame_free(&(ctx->frame));

           }

           if (ctx->pkt) {

               av_packet_free(&(ctx->pkt));

           }

           WEBRTC_LOG("Close encoder context and release context, frame, packet", INFO);

           delete ctx;

       }

   }

