#include "main.hpp"
#include "Hooks.hpp"
#include "AccSaberAPI.hpp"
#include "custom-types/shared/register.hpp"
#include "ResultsViewController.hpp"
#include "MenuPanelController.hpp"

#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

// QPM / Scotland2 entry points
extern "C" __attribute__((visibility("default"))) void setup(CModInfo* info) {
    info->id      = MOD_ID;
    info->version = VERSION;
    info->version_long = 0;
    LOG_INFO("AccSaberQuest setup complete");
}

extern "C" __attribute__((visibility("default"))) void load() {
    LOG_INFO("AccSaberQuest loading...");

    // Register custom types (BSML UI controllers)
    custom_types::Register::AutoRegister();

    // Install game hooks
    AccSaberQuest::Hooks::InstallHooks();

    // Begin in-game authentication (non-blocking)
    AccSaberAPI::AuthenticateIngame([](bool success, std::string userId) {
        if (success) {
            LOG_INFO("AccSaber auth OK – user {}", userId);
        } else {
            LOG_WARN("AccSaber auth failed – running in unauthenticated mode");
        }
    });

    LOG_INFO("AccSaberQuest loaded!");
}
