#include "Hooks.hpp"
#include "main.hpp"
#include "AccSaberAPI.hpp"
#include "ResultsViewController.hpp"
#include "MenuPanelController.hpp"

#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

// Game types (from bs-cordl codegen)
#include "GlobalNamespace/LevelCompletionResults.hpp"
#include "GlobalNamespace/StandardLevelScenesTransitionSetupDataSO.hpp"
#include "GlobalNamespace/IDifficultyBeatmap.hpp"
#include "GlobalNamespace/IBeatmapLevel.hpp"
#include "GlobalNamespace/IBeatmapLevelData.hpp"
#include "GlobalNamespace/ResultsViewController.hpp"
#include "GlobalNamespace/MainMenuViewController.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "HMUI/ViewController.hpp"
#include "bsml/shared/BSML-Lite.hpp"

using namespace GlobalNamespace;
using namespace UnityEngine;

// ─── Persistent pointers ──────────────────────────────────────────────────
static AccSaberQuest::ResultsViewController* g_resultsVC  = nullptr;
static AccSaberQuest::MenuPanelController*   g_menuPanel  = nullptr;

// ─── Hook: ResultsViewController – shown when you finish a song ───────────

MAKE_HOOK_MATCH(
    ResultsVC_DidActivate,
    &GlobalNamespace::ResultsViewController::DidActivate,
    void,
    GlobalNamespace::ResultsViewController* self,
    bool firstActivation,
    bool addedToHierarchy,
    bool screenSystemEnabling
) {
    ResultsVC_DidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);

    if (!firstActivation) return;

    // Inject our custom AccSaber panel under the results screen
    auto* go = GameObject::New_ctor(il2cpp_utils::newcsstr("AccSaberResultsPanel"));
    go->get_transform()->SetParent(self->get_transform(), false);
    g_resultsVC = go->AddComponent<AccSaberQuest::ResultsViewController*>();
}

// ─── Hook: StandardLevelScenesTransitionSetupDataSO – captures song info ─

static std::string g_currentSongHash = "";
static std::string g_currentDiffId   = "";

MAKE_HOOK_MATCH(
    LevelTransitionSetup_Init,
    &StandardLevelScenesTransitionSetupDataSO::Init,
    void,
    StandardLevelScenesTransitionSetupDataSO* self,
    StringW gameMode,
    IDifficultyBeatmap* difficultyBeatmap,
    IPreviewBeatmapLevel* previewBeatmapLevel,
    OverrideEnvironmentSettings* overrideEnvironmentSettings,
    ColorScheme* overrideColorScheme,
    ColorScheme* beatmapOverrideColorScheme,
    GameplayModifiers* gameplayModifiers,
    PlayerSpecificSettings* playerSpecificSettings,
    PracticeSettings* practiceSettings,
    StringW backButtonText,
    bool useTestNoteCutSoundEffects,
    bool startPaused,
    BeatmapDataCache* beatmapDataCache,
    ::System::Nullable_1<RecordingToolManager::SetupData> recordingToolData
) {
    LevelTransitionSetup_Init(self, gameMode, difficultyBeatmap, previewBeatmapLevel,
        overrideEnvironmentSettings, overrideColorScheme, beatmapOverrideColorScheme,
        gameplayModifiers, playerSpecificSettings, practiceSettings, backButtonText,
        useTestNoteCutSoundEffects, startPaused, beatmapDataCache, recordingToolData);

    if (previewBeatmapLevel) {
        // levelID is "custom_level_<HASH>" for custom songs
        std::string levelId = to_utf8(csstrtostr(previewBeatmapLevel->get_levelID()));
        if (levelId.starts_with("custom_level_")) {
            g_currentSongHash = levelId.substr(13); // strip "custom_level_"
            // normalise to uppercase
            std::transform(g_currentSongHash.begin(), g_currentSongHash.end(),
                           g_currentSongHash.begin(), ::toupper);
            LOG_DEBUG("Captured song hash: {}", g_currentSongHash);
        } else {
            g_currentSongHash = "";
        }
    }
}

// ─── Hook: LevelCompletionResults – score is finalised ────────────────────

MAKE_HOOK_MATCH(
    LevelCompletion_Start,
    &GlobalNamespace::ResultsViewController::SetDataToUI,
    void,
    GlobalNamespace::ResultsViewController* self
) {
    LevelCompletion_Start(self);

    if (!g_resultsVC || g_currentSongHash.empty()) return;

    // Kick off AccSaber data fetch
    g_resultsVC->ShowResults(g_currentSongHash, g_currentDiffId, 0.0f);
}

// ─── Hook: MainMenuViewController – inject our AP panel ───────────────────

MAKE_HOOK_MATCH(
    MainMenuVC_DidActivate,
    &MainMenuViewController::DidActivate,
    void,
    MainMenuViewController* self,
    bool firstActivation,
    bool addedToHierarchy,
    bool screenSystemEnabling
) {
    MainMenuVC_DidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);

    if (!firstActivation) return;

    auto* go = GameObject::New_ctor(il2cpp_utils::newcsstr("AccSaberMenuPanel"));
    go->get_transform()->SetParent(self->get_transform(), false);
    g_menuPanel = go->AddComponent<AccSaberQuest::MenuPanelController*>();
}

// ─── Install all hooks ────────────────────────────────────────────────────

void AccSaberQuest::Hooks::InstallHooks() {
    INSTALL_HOOK(MOD_LOGGER, ResultsVC_DidActivate);
    INSTALL_HOOK(MOD_LOGGER, LevelTransitionSetup_Init);
    INSTALL_HOOK(MOD_LOGGER, LevelCompletion_Start);
    INSTALL_HOOK(MOD_LOGGER, MainMenuVC_DidActivate);
    LOG_INFO("All AccSaberQuest hooks installed");
}
