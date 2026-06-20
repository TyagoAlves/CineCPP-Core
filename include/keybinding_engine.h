// AGENT = Agente_13: UX/UI (keyboard mapping profiles)
// AGENT = Agente_08: QA (latency testing)
#ifndef CINECPP_KEYBINDING_ENGINE_H
#define CINECPP_KEYBINDING_ENGINE_H
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>

namespace cinecpp {
struct KeyBinding {
    std::string key;
    std::string action;
    std::string mod;
    std::string description;
};

struct KeyProfile {
    std::string name;
    std::string version;
    std::vector<KeyBinding> bindings;
};

class KeybindingEngine {
public:
    KeybindingEngine();
    bool loadFromFile(const std::string& filepath);
    bool loadFromJSON(const std::string& json);
    std::string resolveAction(const std::string& key, const std::string& mod = "") const;
    const KeyProfile& profile() const { return profile_; }
    std::vector<KeyBinding> bindings() const { return profile_.bindings; }
    size_t bindingCount() const { return profile_.bindings.size(); }
    long long lastLookupMicros() const { return last_lookup_; }

private:
    KeyProfile profile_;
    std::unordered_map<std::string, std::string> lookup_;
    mutable long long last_lookup_ = 0;
};
}
#endif
