#pragma once

#include "custom-types/shared/macros.hpp"
#include "bsml/shared/BSML.hpp"
#include "AccSaberAPI.hpp"
#include <optional>

DECLARE_CLASS_CODEGEN(AccSaberQuest, ResultsViewController, UnityEngine::MonoBehaviour,

    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, apText);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, rankText);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, accuracyText);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, leaderboardText);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, statusText);
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, loadingSpinner);
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, contentRoot);

    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_INSTANCE_METHOD(void, OnEnable);

public:
    // Called from the hook after level completion to populate the panel.
    void ShowResults(
        const std::string& songHash,
        const std::string& difficultyId,
        float scorePercentage
    );

    void SetLoading(bool loading);
    void SetError(const std::string& msg);
    void SetScore(const AccSaberMapScore& score, const std::vector<AccSaberScoreEntry>& lb);
    void SetUnranked();
)
