// AGENT = Agente_13: UX/UI (keyboard keymaps, JSON profile engine)
// AGENT = Agente_08: QA (latency benchmarking)
#include "keybinding_engine.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <chrono>

namespace cinecpp {

KeybindingEngine::KeybindingEngine() {}

static std::string trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static std::string extractString(const std::string& json, const std::string& key) {
    auto pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return "";
    pos = json.find(":", pos + key.size() + 2);
    if (pos == std::string::npos) return "";
    pos = json.find("\"", pos);
    if (pos == std::string::npos) return "";
    auto end = json.find("\"", pos + 1);
    if (end == std::string::npos) return "";
    return json.substr(pos + 1, end - pos - 1);
}

static std::vector<std::string> extractBindings(const std::string& json) {
    std::vector<std::string> bindings;
    auto pos = json.find("\"bindings\"");
    if (pos == std::string::npos) return bindings;
    pos = json.find("[", pos);
    if (pos == std::string::npos) return bindings;
    int depth = 1;
    auto start = pos + 1;
    while (depth > 0 && pos < json.size()) {
        pos++;
        if (json[pos] == '{') depth++;
        else if (json[pos] == '}') depth--;
    }
    std::string array = json.substr(start, pos - start);

    // Extract each object
    pos = 0;
    while (true) {
        auto ob = array.find("{", pos);
        if (ob == std::string::npos) break;
        auto cb = array.find("}", ob);
        if (cb == std::string::npos) break;
        bindings.push_back(array.substr(ob, cb - ob + 1));
        pos = cb + 1;
    }
    return bindings;
}

bool KeybindingEngine::loadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;
    std::stringstream buffer;
    buffer << file.rdbuf();
    return loadFromJSON(buffer.str());
}

bool KeybindingEngine::loadFromJSON(const std::string& json) {
    lookup_.clear();
    profile_.bindings.clear();

    profile_.name = extractString(json, "profile");
    profile_.version = extractString(json, "version");

    auto binding_strings = extractBindings(json);
    for (auto& bs : binding_strings) {
        KeyBinding kb;
        kb.key = extractString(bs, "key");
        kb.action = extractString(bs, "action");
        kb.mod = extractString(bs, "mod");
        kb.description = extractString(bs, "desc");
        profile_.bindings.push_back(kb);

        std::string lookup_key = kb.mod.empty() ? kb.key : kb.mod + "+" + kb.key;
        lookup_[lookup_key] = kb.action;
    }
    return true;
}

std::string KeybindingEngine::resolveAction(const std::string& key, const std::string& mod) const {
    auto start = std::chrono::high_resolution_clock::now();

    std::string lookup_key = mod.empty() ? key : mod + "+" + key;
    auto it = lookup_.find(lookup_key);
    std::string result;
    if (it != lookup_.end()) {
        result = it->second;
    } else {
        // Try without modifier
        it = lookup_.find(key);
        if (it != lookup_.end()) result = it->second;
    }

    auto end = std::chrono::high_resolution_clock::now();
    last_lookup_ = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    return result;
}
}
