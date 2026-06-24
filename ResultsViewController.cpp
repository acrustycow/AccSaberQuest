#include "ResultsViewController.hpp"
#include "AccSaberAPI.hpp"
#include "main.hpp"

#include "bsml/shared/BSML-Lite.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Color.hpp"

DEFINE_TYPE(AccSaberQuest, ResultsViewController);

using namespace UnityEngine;
using namespace BSML::Lite;

void AccSaberQuest::ResultsViewController::Awake() {
    // ── Build UI ──────────────────────────────────────────────────────────
    auto* canvas = CreateCanvas();
    auto* rt     = canvas->GetComponent<RectTransform*>();
    rt->SetParent(get_transform(), false);
    rt->set_anchoredPosition({80.0f, -10.0f});  // right side of results screen
    rt->set_sizeDelta({55.0f, 60.0f});

    // Background
    auto* bg = CreateRoundedRect(canvas->get_transform(), {0, 0}, {55, 60}, 2.0f, Color(0.05f, 0.05f, 0.1f, 0.85f));

    contentRoot = CreateGameObject("Content");
    contentRoot->get_transform()->SetParent(canvas->get_transform(), false);

    // Title
    CreateText(contentRoot->get_transform(), "AccSaber Reloaded", Vector2(0, 25), Vector2(55, 8))
        ->set_fontSize(4.5f);

    // Stats labels
    apText         = CreateText(contentRoot->get_transform(), "AP: --",        {0, 14},  {55, 7});
    accuracyText   = CreateText(contentRoot->get_transform(), "Accuracy: --",  {0,  7},  {55, 7});
    rankText       = CreateText(contentRoot->get_transform(), "Rank: --",      {0,  0},  {55, 7});

    leaderboardText = CreateText(contentRoot->get_transform(), "",             {0, -20}, {55, 28});
    leaderboardText->set_fontSize(3.0f);

    statusText = CreateText(contentRoot->get_transform(), "Loading...", {0, 14}, {55, 7});
    statusText->get_gameObject()->SetActive(false);

    // Loading indicator (simple text spinner for now)
    loadingSpinner = CreateText(contentRoot->get_transform(), "⏳ Fetching AccSaber data...", {0, 0}, {55, 10})
        ->get_gameObject();

    SetLoading(true);
}

void AccSaberQuest::ResultsViewController::OnEnable() {}

void AccSaberQuest::ResultsViewController::SetLoading(bool loading) {
    if (loadingSpinner) loadingSpinner->SetActive(loading);
    if (contentRoot)    contentRoot->SetActive(!loading);
}

void AccSaberQuest::ResultsViewController::SetError(const std::string& msg) {
    SetLoading(false);
    if (statusText) {
        statusText->get_gameObject()->SetActive(true);
        statusText->set_text(il2cpp_utils::newcsstr(msg));
    }
}

void AccSaberQuest::ResultsViewController::SetUnranked() {
    SetLoading(false);
    if (statusText) {
        statusText->get_gameObject()->SetActive(true);
        statusText->set_text(il2cpp_utils::newcsstr("Not ranked on AccSaber"));
    }
    if (apText)       apText->get_gameObject()->SetActive(false);
    if (accuracyText) accuracyText->get_gameObject()->SetActive(false);
    if (rankText)     rankText->get_gameObject()->SetActive(false);
}

void AccSaberQuest::ResultsViewController::SetScore(
    const AccSaberMapScore& score,
    const std::vector<AccSaberScoreEntry>& lb
) {
    SetLoading(false);

    char buf[128];

    snprintf(buf, sizeof(buf), "AP: %.2f", score.ap);
    apText->set_text(il2cpp_utils::newcsstr(buf));

    snprintf(buf, sizeof(buf), "Accuracy: %.2f%%", score.accuracy * 100.0f);
    accuracyText->set_text(il2cpp_utils::newcsstr(buf));

    snprintf(buf, sizeof(buf), "Rank: #%d", score.rank);
    rankText->set_text(il2cpp_utils::newcsstr(buf));

    // Build a mini leaderboard string (top 5)
    std::string lbStr = "── Top Scores ──\n";
    int shown = 0;
    for (auto& e : lb) {
        if (shown++ >= 5) break;
        char line[128];
        snprintf(line, sizeof(line), "#%d %s  %.2f AP\n",
                 e.rank, e.playerName.c_str(), e.ap);
        lbStr += line;
    }
    leaderboardText->set_text(il2cpp_utils::newcsstr(lbStr));
}

void AccSaberQuest::ResultsViewController::ShowResults(
    const std::string& songHash,
    const std::string& /*difficultyId*/,
    float /*scorePercentage*/
) {
    SetLoading(true);

    // Step 1: Check if map is ranked on AccSaber
    AccSaberAPI::GetMapByHash(songHash, [this, songHash](bool ranked, std::string diffId, std::string catId) {
        if (!ranked) {
            SetUnranked();
            return;
        }

        // Step 2: Fetch player's score and leaderboard in parallel
        struct FetchState {
            std::optional<AccSaberMapScore>      myScore;
            std::optional<std::vector<AccSaberScoreEntry>> leaderboard;
            std::mutex mtx;
            int done = 0;
        };
        auto state = std::make_shared<FetchState>();

        auto tryFinish = [this, state]() {
            std::lock_guard<std::mutex> lock(state->mtx);
            state->done++;
            if (state->done < 2) return;

            // Both requests complete
            if (!state->myScore.has_value()) {
                SetError("No AccSaber score yet for this map");
                return;
            }
            SetScore(*state->myScore, state->leaderboard.value_or(std::vector<AccSaberScoreEntry>{}));
        };

        AccSaberAPI::GetMyScoreForMap(songHash, [state, tryFinish](auto score) {
            state->myScore = score;
            tryFinish();
        });

        AccSaberAPI::GetDifficultyLeaderboard(diffId, [state, tryFinish](auto lb) {
            state->leaderboard = lb;
            tryFinish();
        });
    });
}
