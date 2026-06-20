#include "gui.h"
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <sstream>
#include <cstdio>

namespace cinecpp {

static constexpr auto GLSL_VERSION = "#version 130";
static constexpr float MENUBAR_HEIGHT = 20.0f;
static constexpr float TOOLBAR_HEIGHT = 40.0f;
static constexpr float TRANSPORT_HEIGHT = 48.0f;
static constexpr float STATUSBAR_HEIGHT = 24.0f;
static constexpr float TIMELINE_DEFAULT_HEIGHT = 250.0f;
static constexpr float CLIP_HEIGHT = 40.0f;
static constexpr float TRACK_LABEL_WIDTH = 120.0f;
static constexpr float RULER_HEIGHT = 25.0f;
static constexpr float MEDIA_BROWSER_WIDTH = 220.0f;
static constexpr float INFO_PANEL_WIDTH = 250.0f;

CineCPPGUI::CineCPPGUI() {
    auto dir = std::string(getenv("HOME")) + "/cinecpp/keymaps/";
    input_mgr_.loadProfileFromFile(dir + "adobe_premiere.json");
    profiles_ = input_mgr_.availableProfiles();

    timeline_.addTrack("Video 1");
    timeline_.addTrack("Video 2");
    timeline_.addTrack("Audio 1");
    timeline_.addTrack("Audio 2");

    timeline_.addClip(1, "/media/demo/clip1.mp4", 0.0, 5.0);
    timeline_.addClip(1, "/media/demo/clip2.mp4", 5.0, 3.5);
    timeline_.addClip(2, "/media/demo/overlay.mp4", 2.0, 4.0);
    timeline_.addClip(3, "/media/demo/audio_track.wav", 0.0, 8.0);

    status_message_ = "Pronto";
}

CineCPPGUI::~CineCPPGUI() {
    shutdown();
}

bool CineCPPGUI::init(const char* title, int width, int height) {
    if (!initSDL(width, height)) return false;
    if (!initImGui()) return false;
    setupTheme();
    status_message_ = "GUI inicializada - Pronto para editar";
    return true;
}

bool CineCPPGUI::initSDL(int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::fprintf(stderr, "SDL_Init falhou: %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    window_ = SDL_CreateWindow(
        "CineCPP Editor de Video v0.1",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
    );
    if (!window_) {
        std::fprintf(stderr, "SDL_CreateWindow falhou: %s\n", SDL_GetError());
        return false;
    }

    gl_context_ = SDL_GL_CreateContext(window_);
    if (!gl_context_) {
        std::fprintf(stderr, "SDL_GL_CreateContext falhou: %s\n", SDL_GetError());
        return false;
    }
    SDL_GL_MakeCurrent(window_, gl_context_);
    SDL_GL_SetSwapInterval(1);
    return true;
}

bool CineCPPGUI::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.IniFilename = nullptr;

    if (!ImGui_ImplSDL2_InitForOpenGL(window_, gl_context_)) return false;
    if (!ImGui_ImplOpenGL3_Init(GLSL_VERSION)) return false;
    return true;
}

void CineCPPGUI::setupTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.FrameRounding = 2.0f;
    style.GrabRounding = 2.0f;
    style.PopupRounding = 2.0f;
    style.ScrollbarRounding = 2.0f;
    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.TabBorderSize = 0.0f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.10f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.11f, 0.11f, 0.12f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.10f, 0.11f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.27f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.25f, 0.28f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.30f, 0.30f, 0.33f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.30f, 0.32f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.35f, 0.38f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.22f, 0.22f, 0.25f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.16f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.35f, 0.60f, 0.90f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.40f, 0.70f, 1.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.35f, 0.60f, 0.90f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.35f, 0.60f, 0.90f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.25f, 0.25f, 0.28f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.35f, 0.35f, 0.38f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.45f, 0.45f, 0.48f, 1.00f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.35f, 0.60f, 0.90f, 0.50f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.35f, 0.60f, 0.90f, 0.80f);
}

void CineCPPGUI::run() {
    auto last_time = std::chrono::steady_clock::now();
    auto frame_start = last_time;

    while (running_) {
        frame_start = std::chrono::steady_clock::now();
        handleEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        renderFrame();

        ImGui::Render();
        int display_w, display_h;
        SDL_GL_GetDrawableSize(window_, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.09f, 0.09f, 0.10f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            SDL_Window* backup_current = SDL_GL_GetCurrentWindow();
            SDL_GLContext backup_current_ctx = SDL_GL_GetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            SDL_GL_MakeCurrent(backup_current, backup_current_ctx);
        }

        SDL_GL_SwapWindow(window_);

        if (transport_.playing) {
            auto now = std::chrono::steady_clock::now();
            double dt = std::chrono::duration<double>(now - last_time).count();
            transport_.current_time += dt;
            double total = timeline_.duration();
            if (transport_.current_time > total) {
                if (transport_.loop)
                    transport_.current_time = 0.0;
                else {
                    transport_.current_time = total;
                    transport_.playing = false;
                }
            }
            last_time = now;
        } else {
            last_time = std::chrono::steady_clock::now();
        }

        auto elapsed = std::chrono::steady_clock::now() - frame_start;
        auto sleep_ms = std::chrono::duration<double, std::milli>(elapsed).count();
        if (sleep_ms < 16.0) {
            SDL_Delay(static_cast<Uint32>(16.0 - sleep_ms));
        }
    }
}

void CineCPPGUI::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) running_ = false;
        if (event.type == SDL_WINDOWEVENT &&
            event.window.event == SDL_WINDOWEVENT_CLOSE) running_ = false;

        if (event.type == SDL_KEYDOWN && !ImGui::GetIO().WantCaptureKeyboard) {
            int mod = 0;
            if (event.key.keysym.mod & KMOD_CTRL) mod |= 1;
            if (event.key.keysym.mod & KMOD_SHIFT) mod |= 2;
            if (event.key.keysym.mod & KMOD_ALT) mod |= 4;

            switch (event.key.keysym.sym) {
                case SDLK_SPACE:
                    transport_.playing = !transport_.playing;
                    status_message_ = transport_.playing ? "Reproduzindo" : "Pausado";
                    break;
                case SDLK_LEFT:
                    transport_.current_time = std::max(0.0, transport_.current_time - 1.0 / fps_);
                    break;
                case SDLK_RIGHT:
                    transport_.current_time = std::min(timeline_.duration(),
                        transport_.current_time + 1.0 / fps_);
                    break;
                case SDLK_HOME:
                    transport_.current_time = 0.0;
                    break;
                case SDLK_END:
                    transport_.current_time = timeline_.duration();
                    break;
                default: break;
            }
        }
    }
}

void CineCPPGUI::renderFrame() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("DockSpace", nullptr, flags);
    ImGui::PopStyleVar(2);

    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));

    if (ImGui::BeginMenuBar()) {
        renderMenuBar();
        ImGui::EndMenuBar();
    }
    ImGui::End();

    renderToolbar();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
    renderMediaBrowser();
    renderPreview();
    renderTimeline();
    renderInfoPanel();
    ImGui::PopStyleVar();

    renderTransportBar();
    renderStatusBar();
}

void CineCPPGUI::renderMenuBar() {
    if (ImGui::BeginMenu("Arquivo")) {
        if (ImGui::MenuItem("Novo Projeto", "Ctrl+N")) {
            timeline_.clear();
            transport_.current_time = 0.0;
            transport_.playing = false;
            media_files_.clear();
            status_message_ = "Novo projeto criado";
        }
        if (ImGui::MenuItem("Abrir Projeto", "Ctrl+O")) {
            status_message_ = "Abrir projeto - funcionalidade futura";
        }
        if (ImGui::MenuItem("Salvar Projeto", "Ctrl+S")) {
            status_message_ = "Projeto salvo";
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Importar Midia", "Ctrl+I")) {
            ImGui::OpenPopup("Importar Midia");
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Sair", "Alt+F4")) {
            running_ = false;
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Editar")) {
        if (ImGui::MenuItem("Desfazer", "Ctrl+Z")) {}
        if (ImGui::MenuItem("Refazer", "Ctrl+Shift+Z")) {}
        ImGui::Separator();
        if (ImGui::MenuItem("Recortar", "Ctrl+X")) {}
        if (ImGui::MenuItem("Copiar", "Ctrl+C")) {}
        if (ImGui::MenuItem("Colar", "Ctrl+V")) {}
        ImGui::Separator();
        if (ImGui::MenuItem("Dividir Clip", "Ctrl+K")) {}
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Exibir")) {
        if (ImGui::MenuItem("Linha do Tempo", "", true)) {}
        if (ImGui::MenuItem("Navegador de Midia", "", true)) {}
        if (ImGui::MenuItem("Propriedades", "", true)) {}
        ImGui::Separator();
        if (ImGui::MenuItem("Zoom +", "=")) transport_.zoom *= 1.5f;
        if (ImGui::MenuItem("Zoom -", "-")) transport_.zoom = (std::max)(0.1, transport_.zoom / 1.5);
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Perfis")) {
        for (size_t i = 0; i < profiles_.size(); i++) {
            bool selected = (i == static_cast<size_t>(active_profile_idx_));
            if (ImGui::MenuItem(profiles_[i].c_str(), nullptr, &selected)) {
                active_profile_idx_ = i;
                input_mgr_.loadProfile(profiles_[i]);
                status_message_ = "Perfil ativo: " + profiles_[i];
            }
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Ajuda")) {
        if (ImGui::MenuItem("Sobre CineCPP")) {
            status_message_ = "CineCPP v0.1 - Editor de Video C++20";
        }
        if (ImGui::MenuItem("Atalhos de Teclado")) {
            status_message_ = "Espaco=Play/Pause | Setas=Navegar | Home/End=Inicio/Fim";
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginPopup("Importar Midia")) {
        ImGui::Text("Importar arquivo de midia");
        ImGui::Separator();
        ImGui::InputText("Caminho", import_path_, IM_ARRAYSIZE(import_path_));
        if (ImGui::Button("Importar", ImVec2(120, 0))) {
            if (strlen(import_path_) > 0) {
                addMediaFile(import_path_);
                import_path_[0] = '\0';
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    }
}

void CineCPPGUI::renderToolbar() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 4));
    ImGui::Begin("Toolbar", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse);

    float avail = ImGui::GetContentRegionAvail().x;
    float btn_size = 32.0f;

    auto tooltip = [](const char* text) {
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", text);
    };

    if (ImGui::Button("Novo", ImVec2(btn_size, btn_size))) tooltip("Novo Projeto (Ctrl+N)");
    ImGui::SameLine();
    if (ImGui::Button("Abrir", ImVec2(btn_size, btn_size))) tooltip("Abrir Projeto (Ctrl+O)");
    ImGui::SameLine();
    if (ImGui::Button("Salvar", ImVec2(btn_size, btn_size))) tooltip("Salvar Projeto (Ctrl+S)");
    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();
    if (ImGui::Button("Importar", ImVec2(btn_size, btn_size))) {
        ImGui::OpenPopup("Importar Midia");
        ImGui::SetWindowFocus("Importar Midia");
    }
    tooltip("Importar Midia (Ctrl+I)");
    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();
    if (ImGui::Button("Cortar", ImVec2(btn_size, btn_size))) tooltip("Dividir Clip (Ctrl+K)");
    ImGui::SameLine();
    if (ImGui::Button("Desfazer", ImVec2(btn_size, btn_size))) tooltip("Desfazer (Ctrl+Z)");
    ImGui::SameLine();
    if (ImGui::Button("Refazer", ImVec2(btn_size, btn_size))) tooltip("Refazer (Ctrl+Shift+Z)");

    ImGui::SameLine();
    ImGui::SetCursorPosX(avail - 120);
    ImGui::Text("Perfil: %s", profiles_[active_profile_idx_].c_str());

    ImGui::End();
    ImGui::PopStyleVar();
}

void CineCPPGUI::renderMediaBrowser() {
    ImGui::Begin("Navegador de Midia", nullptr,
        ImGuiWindowFlags_NoFocusOnAppearing);

    if (ImGui::Button("+ Importar", ImVec2(-1, 28))) {
        ImGui::OpenPopup("Importar Midia");
    }

    ImGui::Separator();
    ImGui::Text("Arquivos (%zu)", media_files_.size());
    ImGui::Separator();

    ImGui::BeginChild("MediaList", ImVec2(0, 0), false);

    for (size_t i = 0; i < media_files_.size(); i++) {
        auto& mf = media_files_[i];

        char label[64];
        snprintf(label, sizeof(label), "%s###media_%zu", mf.name.c_str(), i);

        ImGui::PushID(static_cast<int>(i));
        ImVec4 col = (mf.type == "video") ? ImVec4(0.3f, 0.6f, 1.0f, 1.0f) :
                     (mf.type == "audio") ? ImVec4(0.3f, 1.0f, 0.4f, 1.0f) :
                                            ImVec4(0.9f, 0.7f, 0.3f, 1.0f);
        ImGui::TextColored(col, "%s", (mf.type == "video") ? "[V]" :
                                      (mf.type == "audio") ? "[A]" : "[I]");
        ImGui::SameLine();

        if (ImGui::Selectable(label, false, ImGuiSelectableFlags_AllowDoubleClick)) {
            if (ImGui::IsMouseDoubleClicked(0)) {
                int track_id = (mf.type == "audio") ? 3 : 1;
                double dur = (mf.duration > 0) ? mf.duration : 5.0;
                auto last_clips = timeline_.clipsAt(transport_.current_time);
                double start = transport_.current_time;
                if (!last_clips.empty()) {
                    for (auto& c : last_clips) start = std::max(start, c.start_time + c.duration);
                }
                int cid = timeline_.addClip(track_id, mf.filepath, start, dur);
                if (cid > 0) {
                    status_message_ = "Clip adicionado: " + mf.name;
                    selected_clip_id_ = cid;
                }
            }
        }
        if (ImGui::IsItemHovered()) {
            char tip[128];
            snprintf(tip, sizeof(tip), "%s\nDuracao: %.1fs\nTipo: %s",
                     mf.filepath.c_str(), mf.duration, mf.type.c_str());
            ImGui::SetTooltip("%s", tip);
        }
        ImGui::PopID();
    }

    ImGui::EndChild();
    ImGui::End();
}

void CineCPPGUI::renderPreview() {
    ImGui::Begin("Preview", nullptr, ImGuiWindowFlags_NoFocusOnAppearing);

    ImVec2 avail = ImGui::GetContentRegionAvail();
    avail.y -= 4.0f;

    float preview_aspect = 16.0f / 9.0f;
    float preview_w = avail.x;
    float preview_h = avail.x / preview_aspect;
    if (preview_h > avail.y) {
        preview_h = avail.y;
        preview_w = avail.y * preview_aspect;
    }

    float offset_x = (avail.x - preview_w) * 0.5f;
    float offset_y = (avail.y - preview_h) * 0.5f;

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset_x);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offset_y);

    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 p_min = ImGui::GetCursorScreenPos();
    ImVec2 p_max = ImVec2(p_min.x + preview_w, p_min.y + preview_h);

    draw->AddRectFilled(p_min, p_max, IM_COL32(20, 20, 25, 255));
    draw->AddRect(p_min, p_max, IM_COL32(60, 60, 70, 255));

    char time_text[64];
    double sec = transport_.current_time;
    int h = static_cast<int>(sec / 3600);
    int m = static_cast<int>(sec / 60) % 60;
    int s = static_cast<int>(sec) % 60;
    int ms = static_cast<int>((sec - static_cast<int>(sec)) * 100);
    snprintf(time_text, sizeof(time_text), "%02d:%02d:%02d.%02d", h, m, s, ms);
    draw->AddText(ImVec2(p_min.x + 10, p_min.y + 10),
                  IM_COL32(200, 200, 200, 200), time_text);

    if (!transport_.playing) {
        const char* paused = "PAUSADO";
        float tw = ImGui::CalcTextSize(paused).x;
        draw->AddText(ImVec2(p_min.x + (preview_w - tw) * 0.5f, p_min.y + preview_h * 0.5f - 10),
                      IM_COL32(255, 255, 255, 150), paused);
    }

    ImGui::Dummy(ImVec2(preview_w, preview_h));
    ImGui::End();
}

void CineCPPGUI::renderTimeline() {
    ImGui::Begin("Linha do Tempo", nullptr, ImGuiWindowFlags_NoFocusOnAppearing);

    ImVec2 avail = ImGui::GetContentRegionAvail();
    float ruler_end_x = avail.x;

    auto& tracks = timeline_.tracks_;

    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    ImDrawList* draw = ImGui::GetWindowDrawList();
    float total_duration = std::max(timeline_.duration(), 10.0);
    float pixels_per_sec = 80.0f * transport_.zoom;
    float total_width = static_cast<float>(total_duration) * pixels_per_sec;

    ImGui::BeginChild("TimelineRuler", ImVec2(0, RULER_HEIGHT), false);

    auto& style = ImGui::GetStyle();
    float scrollx = ImGui::GetScrollX();

    for (double t = 0; t <= total_duration; t += 1.0) {
        float x = static_cast<float>(t) * pixels_per_sec + TRACK_LABEL_WIDTH - scrollx;
        if (x >= TRACK_LABEL_WIDTH - scrollx && x < ruler_end_x + 50) {
            int sec = static_cast<int>(t);
            int min = sec / 60;
            sec %= 60;
            char buf[16];
            snprintf(buf, sizeof(buf), "%02d:%02d", min, sec);
            draw->AddLine(ImVec2(x, 0), ImVec2(x, RULER_HEIGHT), IM_COL32(80, 80, 90, 255));
            draw->AddText(ImVec2(x + 3, 2), IM_COL32(180, 180, 190, 255), buf);
        }
    }

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + std::max(total_width + TRACK_LABEL_WIDTH, avail.x));
    ImGui::Dummy(ImVec2(1, RULER_HEIGHT));
    ImGui::EndChild();

    float timeline_height = std::max(avail.y - RULER_HEIGHT, CLIP_HEIGHT * 4 + 20);
    ImGui::BeginChild("TimelineTracks", ImVec2(0, timeline_height), false, ImGuiWindowFlags_NoScrollWithMouse);

    scrollx = ImGui::GetScrollX();
    ImVec2 tracks_pos = ImGui::GetCursorScreenPos();

    float track_y = tracks_pos.y;
    for (auto& track : tracks) {
        ImVec2 track_bg_min = ImVec2(tracks_pos.x, track_y);
        ImVec2 track_bg_max = ImVec2(tracks_pos.x + TRACK_LABEL_WIDTH +
            std::max(total_width, avail.x - TRACK_LABEL_WIDTH),
            track_y + CLIP_HEIGHT);

        draw->AddRectFilled(track_bg_min, track_bg_max,
            IM_COL32(15, 15, 18, 255));
        draw->AddRectFilled(
            ImVec2(track_bg_min.x, track_bg_min.y),
            ImVec2(track_bg_min.x + TRACK_LABEL_WIDTH, track_bg_max.y),
            IM_COL32(20, 22, 28, 255));

        ImGui::SetCursorScreenPos(ImVec2(tracks_pos.x, track_y));
        ImGui::PushID(track.id);
        if (ImGui::Selectable("", track.id == selected_track_id_,
                ImGuiSelectableFlags_None, ImVec2(TRACK_LABEL_WIDTH, CLIP_HEIGHT))) {
            selected_track_id_ = track.id;
        }
        ImGui::PopID();

        ImVec2 text_pos = ImVec2(tracks_pos.x + 4, track_y + CLIP_HEIGHT * 0.5f - 8);
        if (track.enabled)
            draw->AddText(text_pos, IM_COL32(180, 180, 190, 255), track.name.c_str());
        else
            draw->AddText(text_pos, IM_COL32(100, 100, 100, 255), track.name.c_str());

        for (auto& clip : track.clips) {
            float clip_x = TRACK_LABEL_WIDTH + static_cast<float>(clip.start_time) * pixels_per_sec - scrollx;
            float clip_w = static_cast<float>(clip.duration) * pixels_per_sec;

            if (clip_x + clip_w < TRACK_LABEL_WIDTH - scrollx) continue;
            if (clip_x > avail.x + scrollx) continue;

            ImVec2 clip_min = ImVec2(clip_x, track_y + 4);
            ImVec2 clip_max = ImVec2(clip_x + clip_w, track_y + CLIP_HEIGHT - 4);

            bool is_selected = (clip.id == selected_clip_id_);
            ImU32 clip_color = is_selected ? IM_COL32(50, 100, 180, 220) : IM_COL32(40, 70, 120, 200);

            if (clip.filepath.find("audio") != std::string::npos ||
                clip.filepath.find(".wav") != std::string::npos ||
                clip.filepath.find(".mp3") != std::string::npos) {
                clip_color = is_selected ? IM_COL32(50, 140, 60, 220) : IM_COL32(40, 100, 50, 200);
            }

            draw->AddRectFilled(clip_min, clip_max, clip_color, 3.0f);

            if (clip_w > 30) {
                char clip_label[128];
                auto pos = clip.filepath.find_last_of("/\\");
                std::string fname = (pos != std::string::npos) ? clip.filepath.substr(pos + 1) : clip.filepath;
                snprintf(clip_label, sizeof(clip_label), "%s [%.1fs]", fname.c_str(), clip.duration);
                float text_w = ImGui::CalcTextSize(clip_label).x;
                if (text_w < clip_w - 8) {
                    draw->AddText(ImVec2(clip_min.x + 4, clip_min.y + 6),
                                  IM_COL32(220, 220, 230, 255), clip_label);
                } else {
                    draw->AddText(ImVec2(clip_min.x + 4, clip_min.y + 6),
                                  IM_COL32(220, 220, 230, 255), fname.c_str());
                }
            }

            ImGui::SetCursorScreenPos(ImVec2(clip_x, track_y + 4));
            ImGui::InvisibleButton("clip", ImVec2(clip_w, CLIP_HEIGHT - 8));
            if (ImGui::IsItemHovered()) {
                char tip[256];
                snprintf(tip, sizeof(tip), "Clip #%d\n%s\nInicio: %.1fs\nFim: %.1fs\nDuracao: %.1fs",
                    clip.id, clip.filepath.c_str(), clip.start_time,
                    clip.start_time + clip.duration, clip.duration);
                ImGui::SetTooltip("%s", tip);
            }
            if (ImGui::IsItemClicked()) {
                selected_clip_id_ = clip.id;
                selected_track_id_ = track.id;
                status_message_ = "Clip #" + std::to_string(clip.id) + " selecionado";
            }
        }

        track_y += CLIP_HEIGHT;
    }

    float playhead_x = TRACK_LABEL_WIDTH +
        static_cast<float>(transport_.current_time) * pixels_per_sec - scrollx;
    draw->AddLine(
        ImVec2(playhead_x, tracks_pos.y),
        ImVec2(playhead_x, track_y),
        IM_COL32(255, 60, 60, 255), 2.0f);

    ImGui::SetCursorPosX(std::max(total_width + TRACK_LABEL_WIDTH, avail.x));
    ImGui::SetCursorPosY(track_y - tracks_pos.y + 10);
    ImGui::Dummy(ImVec2(1, 1));
    ImGui::EndChild();

    ImGui::End();
}

void CineCPPGUI::renderInfoPanel() {
    ImGui::Begin("Propriedades", nullptr, ImGuiWindowFlags_NoFocusOnAppearing);

    if (selected_clip_id_ > 0) {
        Clip* clip = timeline_.findClip(selected_clip_id_);
        if (clip) {
            ImGui::Text("Clip #%d", clip->id);
            ImGui::Separator();
            ImGui::Text("Arquivo: %s", clip->filepath.c_str());
            ImGui::Text("Track: %d", clip->track);
            ImGui::Text("Inicio: %.2fs", clip->start_time);
            ImGui::Text("Duracao: %.2fs", clip->duration);
            ImGui::Text("Fim: %.2fs", clip->start_time + clip->duration);

            ImGui::Separator();
            ImGui::Text("In Point: %.2fs", clip->in_point);
            ImGui::Text("Out Point: %.2fs", clip->out_point);

            ImGui::Separator();
            if (ImGui::Button("Remover Clip", ImVec2(-1, 28))) {
                timeline_.removeClip(clip->id);
                status_message_ = "Clip #" + std::to_string(clip->id) + " removido";
                selected_clip_id_ = -1;
            }
        } else {
            selected_clip_id_ = -1;
        }
    } else {
        ImGui::Text("Nenhum clip selecionado");
        ImGui::Separator();
        ImGui::Text("Clique em um clip na");
        ImGui::Text("linha do tempo para");
        ImGui::Text("ver suas propriedades.");
    }

    ImGui::Separator();
    ImGui::Text("Atalhos Ativos");
    ImGui::Separator();
    auto bindings = input_mgr_.currentProfile().bindings;
    if (ImGui::BeginChild("KeyList", ImVec2(0, 0), false)) {
        for (auto& b : bindings) {
            char label[128];
            snprintf(label, sizeof(label), "%s%s -> %s",
                     b.mod.empty() ? "" : (b.mod + "+").c_str(),
                     b.key.c_str(), b.action.c_str());
            ImGui::BulletText("%s", label);
        }
    }
    ImGui::EndChild();

    ImGui::End();
}

void CineCPPGUI::renderTransportBar() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 6));
    ImGui::Begin("Transporte", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse);

    float avail = ImGui::GetContentRegionAvail().x;
    float btn_size = 36.0f;

    auto tooltip = [](const char* text) {
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", text);
    };

    float start_x = (avail - btn_size * 5 - 40) * 0.5f;
    ImGui::SetCursorPosX(start_x);

    if (ImGui::Button("|<", ImVec2(btn_size, btn_size))) {
        transport_.current_time = 0.0;
        status_message_ = "Inicio da linha do tempo";
    }
    tooltip("Inicio (Home)");
    ImGui::SameLine();

    const char* play_label = transport_.playing ? "||" : ">";
    if (ImGui::Button(play_label, ImVec2(btn_size, btn_size))) {
        transport_.playing = !transport_.playing;
        status_message_ = transport_.playing ? "Reproduzindo" : "Pausado";
    }
    tooltip(transport_.playing ? "Pausar (Espaco)" : "Reproduzir (Espaco)");
    ImGui::SameLine();

    if (ImGui::Button("[]", ImVec2(btn_size, btn_size))) {
        transport_.playing = false;
        transport_.current_time = 0.0;
        status_message_ = "Parado";
    }
    tooltip("Parar");
    ImGui::SameLine();

    if (ImGui::Button("<<", ImVec2(btn_size, btn_size))) {
        transport_.current_time = std::max(0.0, transport_.current_time - 1.0);
        status_message_ = "Frame anterior";
    }
    tooltip("Frame anterior (Seta Esq)");
    ImGui::SameLine();

    if (ImGui::Button(">>", ImVec2(btn_size, btn_size))) {
        transport_.current_time = std::min(timeline_.duration(), transport_.current_time + 1.0);
        status_message_ = "Proximo frame";
    }
    tooltip("Proximo frame (Seta Dir)");
    ImGui::SameLine();
    ImGui::SetCursorPosX(start_x + btn_size * 5 + 30);

    char time_buf[64];
    double sec = transport_.current_time;
    int h = static_cast<int>(sec / 3600);
    int m = static_cast<int>(sec / 60) % 60;
    int s = static_cast<int>(sec) % 60;
    int ms = static_cast<int>((sec - static_cast<int>(sec)) * 100);
    snprintf(time_buf, sizeof(time_buf), "%02d:%02d:%02d.%02d", h, m, s, ms);

    double dur_sec = timeline_.duration();
    int dh = static_cast<int>(dur_sec / 3600);
    int dm = static_cast<int>(dur_sec / 60) % 60;
    int ds = static_cast<int>(dur_sec) % 60;
    char dur_buf[16];
    snprintf(dur_buf, sizeof(dur_buf), "%02d:%02d:%02d", dh, dm, ds);

    ImGui::Text("%s / %s", time_buf, dur_buf);

    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Tempo atual / Duracao total");

    ImGui::SameLine();
    ImGui::SetCursorPosX(avail - 70);
    ImGui::Text("FPS: %.0f", fps_);

    ImGui::End();
    ImGui::PopStyleVar();
}

void CineCPPGUI::renderStatusBar() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 4));
    ImGui::Begin("StatusBar", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse);

    float avail = ImGui::GetContentRegionAvail().x;

    ImGui::Text("%s", status_message_.c_str());
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", status_message_.c_str());

    ImGui::SameLine();
    ImGui::SetCursorPosX(avail - 200);

    int total_clips = 0;
    for (auto& track : timeline_.tracks_)
        total_clips += static_cast<int>(track.clips.size());

    ImGui::Text("Tracks: %zu | Clips: %d | Perfil: %s",
        timeline_.tracks_.size(), total_clips,
        profiles_[active_profile_idx_].c_str());

    ImGui::End();
    ImGui::PopStyleVar();
}

void CineCPPGUI::addMediaFile(const std::string& path) {
    MediaFile mf;
    mf.filepath = path;
    auto pos = path.find_last_of("/\\");
    mf.name = (pos != std::string::npos) ? path.substr(pos + 1) : path;

    auto ext_pos = mf.name.find_last_of(".");
    std::string ext;
    if (ext_pos != std::string::npos) {
        ext = mf.name.substr(ext_pos + 1);
        for (auto& c : ext) c = static_cast<char>(std::tolower(c));
    }

    if (ext == "mp4" || ext == "mov" || ext == "avi" || ext == "mkv" || ext == "webm")
        mf.type = "video";
    else if (ext == "wav" || ext == "mp3" || ext == "flac" || ext == "ogg" || ext == "aac")
        mf.type = "audio";
    else
        mf.type = "image";

    mf.duration = 10.0;

    media_files_.push_back(mf);
    status_message_ = "Midia importada: " + mf.name;
}

void CineCPPGUI::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    if (gl_context_) SDL_GL_DeleteContext(gl_context_);
    if (window_) SDL_DestroyWindow(window_);
    SDL_Quit();
}

}
