// AGENT = Agente_06: Backend Core
#ifndef CINECPP_INPUT_MANAGER_H
#define CINECPP_INPUT_MANAGER_H
#include <string>
#include <functional>
#include <unordered_map>
#include <vector>
#include "keybinding_engine.h"

namespace cinecpp {
class InputManager {
public:
    using ActionCallback = std::function<void(const std::string&)>;

    InputManager();
    ~InputManager();

    bool loadProfile(const std::string& profile_name);
    bool loadProfileFromFile(const std::string& filepath);
    bool handleKeyPress(int key, int mod);
    bool handleKeyPress(const std::string& key_name, const std::string& mod = "");

    void setCallback(ActionCallback cb) { callback_ = cb; }
    void setProfile(KeyProfile profile) { current_profile_ = profile; }
    KeyProfile currentProfile() const { return current_profile_; }
    std::vector<std::string> availableProfiles() const;

private:
    KeybindingEngine engine_;
    KeyProfile current_profile_;
    ActionCallback callback_;
};
}
#endif
