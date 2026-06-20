#ifndef CINECPP_GUI_H
#define CINECPP_GUI_H

#include <string>
#include <vector>
#include <memory>
#include "timeline_engine.h"
#include "input_manager.h"

struct SDL_Window;
struct ImFont;

namespace cinecpp {

struct MediaFile {
    std::string filepath;
    std::string name;
    double duration;
    std::string type;
};

struct TransportState {
    bool playing = false;
    double current_time = 0.0;
    double zoom = 1.0;
    double volume = 1.0;
    bool loop = false;
    bool snap = true;
};

class CineCPPGUI {
public:
    CineCPPGUI();
    ~CineCPPGUI();

    bool init(const char* title, int width, int height);
    void run();
    void shutdown();

private:
    bool initSDL(int width, int height);
    bool initImGui();
    void setupTheme();
    void handleEvents();
    void renderFrame();
    void renderMenuBar();
    void renderToolbar();
    void renderMediaBrowser();
    void renderPreview();
    void renderTimeline();
    void renderInfoPanel();
    void renderTransportBar();
    void renderStatusBar();
    void addMediaFile(const std::string& path);

    SDL_Window* window_ = nullptr;
    void* gl_context_ = nullptr;
    ImFont* font_ = nullptr;
    bool running_ = true;

    TimelineEngine timeline_;
    InputManager input_mgr_;
    TransportState transport_;
    std::vector<MediaFile> media_files_;
    int selected_clip_id_ = -1;
    int selected_track_id_ = -1;
    int active_profile_idx_ = 0;
    std::vector<std::string> profiles_;
    std::string status_message_;
    double fps_ = 30.0;
    char import_path_[256] = "";
    char new_track_name_[64] = "Video Track";
};

}

#endif
