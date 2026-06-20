// AGENT = Agente_06: Backend Core (timeline engine, linked lists)
// AGENT = Agente_14: Threads (concurrency, memory management)
#ifndef CINECPP_TIMELINE_ENGINE_H
#define CINECPP_TIMELINE_ENGINE_H
#include <vector>
#include <memory>
#include <string>
#include <chrono>

namespace cinecpp {
struct Clip {
    int id;
    std::string filepath;
    double start_time;
    double duration;
    double in_point;
    double out_point;
    int track;
    bool enabled;
};

struct Track {
    int id;
    std::string name;
    bool enabled;
    std::vector<Clip> clips;
};

class TimelineEngine {
public:
    TimelineEngine();
    ~TimelineEngine();

    int addTrack(const std::string& name);
    bool removeTrack(int track_id);
    int addClip(int track_id, const std::string& filepath, double start, double duration);
    bool removeClip(int clip_id);
    bool splitClip(int clip_id, double split_time);
    Clip* findClip(int clip_id);
    std::vector<Clip> clipsAt(double time) const;
    double duration() const;
    void clear();

    std::vector<Track> tracks_;
    int next_clip_id_ = 1;
    int next_track_id_ = 1;
};
}
#endif
