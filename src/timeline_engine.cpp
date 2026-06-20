// AGENT = Agente_06: Backend Core (timeline linked list engine)
// AGENT = Agente_14: Threads (concurrency)
#include "timeline_engine.h"
#include <algorithm>

namespace cinecpp {

TimelineEngine::TimelineEngine() {}
TimelineEngine::~TimelineEngine() {}

int TimelineEngine::addTrack(const std::string& name) {
    Track t;
    t.id = next_track_id_++;
    t.name = name;
    t.enabled = true;
    tracks_.push_back(t);
    return t.id;
}

bool TimelineEngine::removeTrack(int track_id) {
    auto it = std::remove_if(tracks_.begin(), tracks_.end(),
        [track_id](const Track& t) { return t.id == track_id; });
    if (it == tracks_.end()) return false;
    tracks_.erase(it, tracks_.end());
    return true;
}

int TimelineEngine::addClip(int track_id, const std::string& filepath, double start, double duration) {
    for (auto& track : tracks_) {
        if (track.id == track_id) {
            Clip c;
            c.id = next_clip_id_++;
            c.filepath = filepath;
            c.start_time = start;
            c.duration = duration;
            c.in_point = 0;
            c.out_point = duration;
            c.track = track_id;
            c.enabled = true;
            track.clips.push_back(c);
            return c.id;
        }
    }
    return -1;
}

bool TimelineEngine::removeClip(int clip_id) {
    for (auto& track : tracks_) {
        auto it = std::remove_if(track.clips.begin(), track.clips.end(),
            [clip_id](const Clip& c) { return c.id == clip_id; });
        if (it != track.clips.end()) {
            track.clips.erase(it, track.clips.end());
            return true;
        }
    }
    return false;
}

bool TimelineEngine::splitClip(int clip_id, double split_time) {
    for (auto& track : tracks_) {
        for (size_t i = 0; i < track.clips.size(); i++) {
            if (track.clips[i].id == clip_id) {
                Clip& original = track.clips[i];
                Clip new_clip = original;
                new_clip.id = next_clip_id_++;
                new_clip.start_time = split_time;
                new_clip.duration = original.duration - (split_time - original.start_time);
                original.duration = split_time - original.start_time;
                track.clips.insert(track.clips.begin() + i + 1, new_clip);
                return true;
            }
        }
    }
    return false;
}

Clip* TimelineEngine::findClip(int clip_id) {
    for (auto& track : tracks_) {
        for (auto& clip : track.clips) {
            if (clip.id == clip_id) return &clip;
        }
    }
    return nullptr;
}

std::vector<Clip> TimelineEngine::clipsAt(double time) const {
    std::vector<Clip> result;
    for (auto& track : tracks_) {
        for (auto& clip : track.clips) {
            if (time >= clip.start_time && time <= clip.start_time + clip.duration) {
                result.push_back(clip);
            }
        }
    }
    return result;
}

double TimelineEngine::duration() const {
    double max = 0;
    for (auto& track : tracks_) {
        for (auto& clip : track.clips) {
            double end = clip.start_time + clip.duration;
            if (end > max) max = end;
        }
    }
    return max;
}

void TimelineEngine::clear() {
    tracks_.clear();
    next_clip_id_ = 1;
    next_track_id_ = 1;
}
}
