// AGENT = Agente_12: Codecs (FFmpeg decoder, video metadata)
#include "ffmpeg_reader.h"
#include <cstring>
#include <iostream>

#ifndef CINECPP_NO_FFMPEG
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}
#endif

namespace cinecpp {

FFmpegReader::FFmpegReader() {
#ifndef CINECPP_NO_FFMPEG
    static bool registered = false;
    if (!registered) {
        avformat_network_init();
        registered = true;
    }
#endif
}

FFmpegReader::~FFmpegReader() {
    close();
}

VideoMetadata FFmpegReader::open(const std::string& filepath) {
    close();
    VideoMetadata meta;
    meta.filepath = filepath;

#ifndef CINECPP_NO_FFMPEG
    AVFormatContext* fmt_ctx = nullptr;
    if (avformat_open_input(&fmt_ctx, filepath.c_str(), nullptr, nullptr) < 0) {
        meta.valid = false;
        return meta;
    }

    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
        avformat_close_input(&fmt_ctx);
        meta.valid = false;
        return meta;
    }

    meta.duration = fmt_ctx->duration > 0 ? (double)fmt_ctx->duration / AV_TIME_BASE : 0;

    for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++) {
        AVStream* stream = fmt_ctx->streams[i];
        AVCodecParameters* codecpar = stream->codecpar;
        if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO && video_stream_idx_ < 0) {
            video_stream_idx_ = i;
            meta.width = codecpar->width;
            meta.height = codecpar->height;
            meta.fps = stream->avg_frame_rate.num > 0 ?
                (double)stream->avg_frame_rate.num / stream->avg_frame_rate.den : 0;
            meta.bitrate = codecpar->bit_rate;
            auto codec = avcodec_find_decoder(codecpar->codec_id);
            if (codec) meta.codec = codec->name;
        }
        if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO && meta.audio_codec.empty()) {
            auto codec = avcodec_find_decoder(codecpar->codec_id);
            if (codec) meta.audio_codec = codec->name;
        }
    }

    fmt_ctx_ = fmt_ctx;
    opened_ = (video_stream_idx_ >= 0);
    meta.valid = opened_;

    if (video_stream_idx_ >= 0) {
        AVCodecParameters* params = fmt_ctx->streams[video_stream_idx_]->codecpar;
        const AVCodec* decoder = avcodec_find_decoder(params->codec_id);
        if (decoder) {
            codec_ctx_ = avcodec_alloc_context3(decoder);
            if (codec_ctx_ && avcodec_parameters_to_context((AVCodecContext*)codec_ctx_, params) >= 0) {
                avcodec_open2((AVCodecContext*)codec_ctx_, decoder, nullptr);
            }
        }
        frame_ = av_frame_alloc();
    }

    metadata_ = meta;
#else
    meta.valid = false;
#endif
    return meta;
}

Frame FFmpegReader::readFrame() {
    Frame f;
#ifndef CINECPP_NO_FFMPEG
    if (!opened_ || !fmt_ctx_) return f;

    AVPacket* packet = av_packet_alloc();
    if (!packet) return f;

    while (av_read_frame((AVFormatContext*)fmt_ctx_, packet) >= 0) {
        if (packet->stream_index == video_stream_idx_) {
            avcodec_send_packet((AVCodecContext*)codec_ctx_, packet);
            int ret = avcodec_receive_frame((AVCodecContext*)codec_ctx_, (AVFrame*)frame_);
            if (ret >= 0) {
                AVFrame* avf = (AVFrame*)frame_;
                f.width = avf->width;
                f.height = avf->height;
                f.pts = avf->pts * av_q2d(((AVFormatContext*)fmt_ctx_)->streams[video_stream_idx_]->time_base);
                size_t size = avf->width * avf->height * 3;
                f.data.resize(size);
                std::memcpy(f.data.data(), avf->data[0], size);
                av_packet_free(&packet);
                return f;
            }
        }
        av_packet_unref(packet);
    }
    av_packet_free(&packet);
#endif
    return f;
}

bool FFmpegReader::seek(double time) {
#ifndef CINECPP_NO_FFMPEG
    if (!opened_ || !fmt_ctx_) return false;
    int64_t ts = (int64_t)(time * AV_TIME_BASE);
    return av_seek_frame((AVFormatContext*)fmt_ctx_, -1, ts, AVSEEK_FLAG_BACKWARD) >= 0;
#else
    return false;
#endif
}

void FFmpegReader::close() {
#ifndef CINECPP_NO_FFMPEG
    if (frame_) av_frame_free((AVFrame**)&frame_);
    frame_ = nullptr;
    if (codec_ctx_) {
        avcodec_close((AVCodecContext*)codec_ctx_);
        avcodec_free_context((AVCodecContext**)&codec_ctx_);
    }
    codec_ctx_ = nullptr;
    if (fmt_ctx_) avformat_close_input((AVFormatContext**)&fmt_ctx_);
    fmt_ctx_ = nullptr;
    video_stream_idx_ = -1;
    opened_ = false;
#endif
}
}
