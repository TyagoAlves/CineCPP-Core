// AGENT = Agente_06: Backend Core (main engine, CLI interface)
// AGENT = Agente_07: Frontend (dashboard visualization)
// AGENT = Agente_20: SRE (watchdog, background monitoring)
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include "input_manager.h"
#include "timeline_engine.h"
#include "ffmpeg_reader.h"
#include "keybinding_engine.h"

void printBanner() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════╗\n";
    std::cout << "║        CineCPP Editor de Video v0.1        ║\n";
    std::cout << "║     C++20 Profissional — Alpine Linux       ║\n";
    std::cout << "╚════════════════════════════════════════════╝\n";
    std::cout << "\n";
}

void printHelp() {
    std::cout << "Comandos:\n";
    std::cout << "  load <arquivo>  — Carregar perfil de teclado\n";
    std::cout << "  list             — Listar bindings ativos\n";
    std::cout << "  key <tecla>      — Simular pressionamento\n";
    std::cout << "  profile          — Mostrar perfil atual\n";
    std::cout << "  test             — Executar benchmark (<1ms)\n";
    std::cout << "  info <video>     — Ler metadados de video\n";
    std::cout << "  exit             — Sair\n";
    std::cout << "\n";
}

int main(int argc, char** argv) {
    printBanner();
    cinecpp::InputManager im;
    cinecpp::TimelineEngine tl;
    cinecpp::FFmpegReader reader;

    auto default_dir = std::string(getenv("HOME")) + "/cinecpp/keymaps/";
    im.loadProfileFromFile(default_dir + "adobe_premiere.json");
    std::cout << "→ Perfil carregado: " << im.currentProfile().name << "\n";

    im.setCallback([](const std::string& action) {
        std::cout << "[Acao] " << action << "\n";
    });

    if (argc > 1) {
        std::string cmd = argv[1];
        if (cmd == "info" && argc > 2) {
            auto meta = reader.open(argv[2]);
            if (meta.valid) {
                std::cout << "Video: " << meta.filepath << "\n";
                std::cout << "  Duracao: " << meta.duration << "s\n";
                std::cout << "  Resolucao: " << meta.width << "x" << meta.height << "\n";
                std::cout << "  FPS: " << meta.fps << "\n";
                std::cout << "  Codec: " << meta.codec << "\n";
            } else {
                std::cout << "Falha ao abrir video\n";
            }
            return 0;
        }
        if (cmd == "test") {
            int n = argc > 2 ? std::stoi(argv[2]) : 10000;
            auto start = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < n; i++) {
                im.handleKeyPress("C");
                im.handleKeyPress("Space");
                im.handleKeyPress("V");
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            double avg = (double)us / (n * 3);
            std::cout << "Benchmark: " << (n*3) << " atalhos em " << us << "us\n";
            std::cout << "Media: " << avg << " us/atalho\n";
            std::cout << (avg < 1.0 ? "[OK] Abaixo de 1ms" : "[ATENCAO] Acima de 1ms") << "\n";
            return 0;
        }
    }

    printHelp();
    std::string line;
    while (true) {
        std::cout << "cinecpp> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "exit") break;
        if (line == "help") { printHelp(); continue; }
        if (line == "list") {
            for (auto& b : im.currentProfile().bindings) {
                std::cout << "  " << (b.mod.empty() ? "" : b.mod + "+") << b.key
                          << " → " << b.action << " (" << b.description << ")\n";
            }
            continue;
        }
        if (line == "profile") {
            std::cout << "Perfil: " << im.currentProfile().name
                      << " (" << im.currentProfile().bindings.size() << " bindings)\n";
            continue;
        }
        if (line.find("load ") == 0) {
            auto path = line.substr(5);
            if (im.loadProfileFromFile(path)) {
                std::cout << "Perfil carregado: " << im.currentProfile().name << "\n";
            } else {
                std::cout << "Falha ao carregar: " << path << "\n";
            }
            continue;
        }
        if (line.find("key ") == 0) {
            auto k = line.substr(4);
            im.handleKeyPress(k);
            continue;
        }
        if (line == "test") {
            auto start = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < 30000; i++) {
                im.handleKeyPress("C");
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            std::cout << "30000 atalhos em " << us << "us (" << (us/30.0) << " ns/atalho)\n";
            continue;
        }
        if (line.find("info ") == 0) {
            auto path = line.substr(5);
            auto meta = reader.open(path);
            if (meta.valid) {
                std::cout << "Video: " << meta.filepath << "\n";
                std::cout << "  Duracao: " << meta.duration << "s\n";
                std::cout << "  Resolucao: " << meta.width << "x" << meta.height << "\n";
                std::cout << "  FPS: " << meta.fps << "\n";
            } else {
                std::cout << "Falha ao abrir (FFmpeg nao disponivel ou arquivo ausente)\n";
            }
            continue;
        }
        std::cout << "Comando nao reconhecido. Digite 'help'.\n";
    }
    std::cout << "Encerrando CineCPP.\n";
    return 0;
}
