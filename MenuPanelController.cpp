#include "MenuPanelController.hpp"
#include "AccSaberAPI.hpp"
#include "main.hpp"

#include "bsml/shared/BSML-Lite.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Color.hpp"

DEFINE_TYPE(AccSaberQuest, MenuPanelController);

using namespace UnityEngine;
using namespace BSML::Lite;

void AccSaberQuest::MenuPanelController::Awake() {
    // Position: bottom-left of the main menu
    auto* canvas = CreateCanvas();
    auto* rt     = canvas->GetComponent<RectTransform*>();
    rt->SetParent(get_transform(), false);
    rt->set_anchoredPosition({-80.0f, -30.0f});
    rt->set_sizeDelta({50.0f, 28.0f});

    // Background
    CreateRoundedRect(canvas->get_transform(), {0, 0}, {50, 28}, 2.0f,
                      Color(0.04f, 0.04f, 0.10f, 0.90f));

    // "Not logged in" panel
    notLoggedInPanel = CreateGameObject("NotLoggedIn");
    notLoggedInPanel->get_transform()->SetParent(canvas->get_transform(), false);
    CreateText(notLoggedInPanel->get_transform(), "AccSaber\nNot logged in", {0, 0}, {50, 28})
        ->set_fontSize(3.5f);

    // Stats panel
    statsPanel = CreateGameObject("Stats");
    statsPanel->get_transform()->SetParent(canvas->get_transform(), false);

    CreateText(statsPanel->get_transform(), "AccSaber Reloaded", {0, 11}, {50, 7})
        ->set_fontSize(4.0f);

    playerNameText = CreateText(statsPanel->get_transform(), "",      {0,  4},  {50, 7});
    apText         = CreateText(statsPanel->get_transform(), "",      {0, -3},  {50, 7});
    rankText       = CreateText(statsPanel->get_transform(), "",      {0, -10}, {50, 7});
    levelText      = CreateText(statsPanel->get_transform(), "",      {0, -17}, {50, 7});

    statsPanel->SetActive(false);
    notLoggedInPanel->SetActive(true);

    Refresh();
}

void AccSaberQuest::MenuPanelController::OnEnable() {
    Refresh();
}

void AccSaberQuest::MenuPanelController::Refresh() {
    AccSaberAPI::GetMyInfo([this](auto infoOpt) {
        if (!infoOpt.has_value()) {
            SetNotLoggedIn();
        } else {
            SetPlayerInfo(*infoOpt);
        }
    });
}

void AccSaberQuest::MenuPanelController::SetPlayerInfo(const AccSaberPlayerInfo& info) {
    char buf[64];

    playerNameText->set_text(il2cpp_utils::newcsstr(info.name));

    snprintf(buf, sizeof(buf), "%.2f AP", info.ap);
    apText->set_text(il2cpp_utils::newcsstr(buf));

    snprintf(buf, sizeof(buf), "Global Rank #%d", info.rank);
    rankText->set_text(il2cpp_utils::newcsstr(buf));

    snprintf(buf, sizeof(buf), "Level %d", info.level);
    levelText->set_text(il2cpp_utils::newcsstr(buf));

    if (statsPanel)        statsPanel->SetActive(true);
    if (notLoggedInPanel)  notLoggedInPanel->SetActive(false);
}

void AccSaberQuest::MenuPanelController::SetNotLoggedIn() {
    if (statsPanel)       statsPanel->SetActive(false);
    if (notLoggedInPanel) notLoggedInPanel->SetActive(true);
}
