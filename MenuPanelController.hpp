#pragma once

#include "custom-types/shared/macros.hpp"
#include "bsml/shared/BSML.hpp"
#include "AccSaberAPI.hpp"

DECLARE_CLASS_CODEGEN(AccSaberQuest, MenuPanelController, UnityEngine::MonoBehaviour,

    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, playerNameText);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, apText);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, rankText);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, levelText);
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, notLoggedInPanel);
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, statsPanel);

    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_INSTANCE_METHOD(void, OnEnable);

public:
    void Refresh();
    void SetPlayerInfo(const AccSaberPlayerInfo& info);
    void SetNotLoggedIn();
)
