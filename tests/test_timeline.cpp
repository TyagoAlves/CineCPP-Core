// AGENT = Agente_08: QA (testes timeline, split, clips)
#include <iostream>
#include <cassert>
#include "timeline_engine.h"

int main() {
    int passed = 0, failed = 0;
    std::cout << "=== Timeline Engine Tests ===\n\n";

    // Test 1: Add tracks and clips
    {
        cinecpp::TimelineEngine tl;
        int t1 = tl.addTrack("Video 1");
        int t2 = tl.addTrack("Audio 1");
        assert(t1 > 0);
        assert(t2 > 0);
        assert(tl.tracks_.size() == 2);
        int c1 = tl.addClip(t1, "/test/video.mp4", 0.0, 10.0);
        int c2 = tl.addClip(t1, "/test/video2.mp4", 10.0, 5.0);
        assert(c1 > 0);
        assert(c2 > 0);
        assert(tl.tracks_[0].clips.size() == 2);
        std::cout << "[PASS] Test 1: Add tracks and clips\n";
        passed++;
    }

    // Test 2: Split clip
    {
        cinecpp::TimelineEngine tl;
        int t = tl.addTrack("V");
        int c = tl.addClip(t, "/test/v.mp4", 0.0, 10.0);
        assert(c > 0);

        bool split = tl.splitClip(c, 4.0);
        assert(split);
        assert(tl.tracks_[0].clips.size() == 2);
        assert(tl.tracks_[0].clips[0].duration == 4.0);
        assert(tl.tracks_[0].clips[1].duration == 6.0);
        std::cout << "[PASS] Test 2: Split clip at 4.0s\n";
        passed++;
    }

    // Test 3: Remove clip
    {
        cinecpp::TimelineEngine tl;
        int t = tl.addTrack("V");
        int c = tl.addClip(t, "/test/v.mp4", 0.0, 10.0);
        bool removed = tl.removeClip(c);
        assert(removed);
        assert(tl.tracks_[0].clips.empty());
        std::cout << "[PASS] Test 3: Remove clip\n";
        passed++;
    }

    // Test 4: Clips at time
    {
        cinecpp::TimelineEngine tl;
        int t = tl.addTrack("V");
        tl.addClip(t, "/test/a.mp4", 0.0, 5.0);
        tl.addClip(t, "/test/b.mp4", 5.0, 5.0);
        auto at_2 = tl.clipsAt(2.0);
        auto at_7 = tl.clipsAt(7.0);
        assert(at_2.size() == 1);
        assert(at_7.size() == 1);
        std::cout << "[PASS] Test 4: Clips at time\n";
        passed++;
    }

    // Test 5: Duration
    {
        cinecpp::TimelineEngine tl;
        int t = tl.addTrack("V");
        tl.addClip(t, "/test/a.mp4", 0.0, 10.0);
        tl.addClip(t, "/test/b.mp4", 30.0, 20.0);
        assert(tl.duration() == 50.0);
        std::cout << "[PASS] Test 5: Duration calculation\n";
        passed++;
    }

    std::cout << "\n=== Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed > 0 ? 1 : 0;
}
