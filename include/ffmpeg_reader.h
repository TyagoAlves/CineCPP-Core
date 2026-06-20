// AGENT = Agente_12: Codecs (FFmpeg integration, video decoding)
#ifndef CINECPP_FFMPEG_READER_H
#define CINECPP_FFMPEG_READER_H
#include <string>
#include <vector>
#include <memory>

namespace cinecpp {
struct VideoMetadata {
    std::string filepath;
    double duration;
    int width;
    int height;
    double fps;
    int64_t bitrate;
    std::string codec;
    std::string audio_codec;
    bool valid = false;
};

struct Frame {
    std::vector<uint8_t> data;
    int width;
    int height;
    double pts;
};

class FFmpegReader {
public:
    FFmpegReader();
    ~FFmpegReader();
    VideoMetadata open(const std::string& filepath);
    Frame readFrame();
    bool seek(double time);
    void close();
    bool isOpen() const { return opened_; }
    VideoMetadata metadata() const { return metadata_; }

private:
    bool opened_ = false;
    VideoMetadata metadata_;
    void* fmt_ctx_ = nullptr;
    int video_stream_idx_ = -1;
    void* codec_ctx_ = nullptr;
    void* frame_ = nullptr;
    void* pkt_ = nullptr;
};
}
#endif
