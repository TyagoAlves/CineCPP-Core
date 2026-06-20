// AGENT = Agente_06: Backend Core (input capture, keystroke translation)
// AGENT = Agente_13: UX/UI (keyboard mapping profiles)
#include "input_manager.h"

namespace cinecpp {
InputManager::InputManager() {
    current_profile_.name = "default";
    current_profile_.version = "1.0";
}

InputManager::~InputManager() {}

bool InputManager::loadProfile(const std::string& profile_name) {
    std::string path = std::string(getenv("HOME")) + "/cinecpp/keymaps/";
    if (profile_name == "adobe" || profile_name == "adobe_premiere")
        return loadProfileFromFile(path + "adobe_premiere.json");
    if (profile_name == "davinci" || profile_name == "davinci_resolve")
        return loadProfileFromFile(path + "davinci_resolve.json");
    if (profile_name == "affinity")
        return loadProfileFromFile(path + "affinity.json");
    return false;
}

bool InputManager::loadProfileFromFile(const std::string& filepath) {
    if (!engine_.loadFromFile(filepath))
        return false;
    current_profile_ = engine_.profile();
    return true;
}

bool InputManager::handleKeyPress(int key, int mod) {
    // Simulated - in real app would read from event system
    return false;
}

bool InputManager::handleKeyPress(const std::string& key_name, const std::string& mod) {
    auto action = engine_.resolveAction(key_name, mod);
    if (!action.empty() && callback_) {
        callback_(action);
        return true;
    }
    return false;
}

std::vector<std::string> InputManager::availableProfiles() const {
    return {"adobe_premiere", "davinci_resolve", "affinity"};
}
}
