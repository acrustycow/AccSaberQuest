#include "AccSaberAPI.hpp"
#include "main.hpp"

#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "GlobalNamespace/UserInfo.hpp"
#include "GlobalNamespace/IPlatformUserModel.hpp"
#include "UnityEngine/Networking/UnityWebRequest.hpp"
#include "UnityEngine/Networking/UnityWebRequestAsyncOperation.hpp"
#include "UnityEngine/Networking/UploadHandlerRaw.hpp"
#include "UnityEngine/Networking/DownloadHandler.hpp"

#include <thread>
#include <sstream>

// rapidjson – included by beatsaber-hook
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

// ─── Static member init ───────────────────────────────────────────────────
std::string AccSaberAPI::authToken      = "";
std::string AccSaberAPI::loggedInUserId = "";

// ─── Helpers ──────────────────────────────────────────────────────────────

static std::string SafeGetString(const rapidjson::Value& v, const char* key, const std::string& def = "") {
    if (v.HasMember(key) && v[key].IsString()) return v[key].GetString();
    return def;
}
static float SafeGetFloat(const rapidjson::Value& v, const char* key, float def = 0.0f) {
    if (v.HasMember(key) && v[key].IsNumber()) return v[key].GetFloat();
    return def;
}
static int SafeGetInt(const rapidjson::Value& v, const char* key, int def = 0) {
    if (v.HasMember(key) && v[key].IsInt()) return v[key].GetInt();
    return def;
}

// ─── HTTP helpers (coroutine-based via UnityWebRequest) ───────────────────

// Fires a GET on the Unity main thread via il2cpp coroutine and calls back on completion.
void AccSaberAPI::HttpGet(
    const std::string& url,
    bool authenticated,
    std::function<void(bool, std::string)> callback
) {
    // We schedule the web request via a coroutine on the main thread.
    // beatsaber-hook provides StartCoroutine-style utilities through il2cpp_utils.
    std::thread([url, authenticated, callback]() {
        // Build the UnityWebRequest on the main thread queue.
        // We use il2cpp_utils::RunMethod to call Unity APIs safely.
        auto* uwr = UnityEngine::Networking::UnityWebRequest::Get(
            il2cpp_utils::newcsstr(url)
        );

        if (authenticated && !AccSaberAPI::authToken.empty()) {
            // Set Authorization header: Bearer <token>
            uwr->SetRequestHeader(
                il2cpp_utils::newcsstr("Authorization"),
                il2cpp_utils::newcsstr("Bearer " + AccSaberAPI::authToken)
            );
        }

        // Send and wait (blocking thread – NOT main thread)
        auto* op = uwr->SendWebRequest();
        while (!op->get_isDone()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        if (uwr->get_isNetworkError() || uwr->get_isHttpError()) {
            LOG_ERROR("HTTP GET {} failed: {}", url, to_utf8(csstrtostr(uwr->get_error())));
            callback(false, "");
        } else {
            std::string body = to_utf8(csstrtostr(uwr->get_downloadHandler()->GetText()));
            callback(true, body);
        }
    }).detach();
}

void AccSaberAPI::HttpPost(
    const std::string& url,
    const std::string& body,
    std::function<void(bool, std::string)> callback
) {
    std::thread([url, body, callback]() {
        auto bodyBytes = il2cpp_utils::newcsstr(body);
        auto* uwr = UnityEngine::Networking::UnityWebRequest::Post(
            il2cpp_utils::newcsstr(url),
            il2cpp_utils::newcsstr("")
        );
        // Attach JSON body
        auto rawBytes = System::Text::Encoding::get_UTF8()->GetBytes(
            il2cpp_utils::newcsstr(body)
        );
        auto* upload = UnityEngine::Networking::UploadHandlerRaw::New_ctor(rawBytes);
        upload->set_contentType(il2cpp_utils::newcsstr("application/json"));
        uwr->set_uploadHandler(upload);
        uwr->SetRequestHeader(
            il2cpp_utils::newcsstr("Content-Type"),
            il2cpp_utils::newcsstr("application/json")
        );

        auto* op = uwr->SendWebRequest();
        while (!op->get_isDone()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        if (uwr->get_isNetworkError() || uwr->get_isHttpError()) {
            LOG_ERROR("HTTP POST {} failed: {}", url, to_utf8(csstrtostr(uwr->get_error())));
            callback(false, "");
        } else {
            std::string resp = to_utf8(csstrtostr(uwr->get_downloadHandler()->GetText()));
            callback(true, resp);
        }
    }).detach();
}

// ─── Auth ─────────────────────────────────────────────────────────────────

void AccSaberAPI::AuthenticateIngame(
    std::function<void(bool, std::string)> callback
) {
    // Grab the Meta/Oculus platform token.
    // On Quest, Unity exposes this through the Social.localUser flow.
    // beatsaber-hook's codegen gives us access to IPlatformUserModel.
    il2cpp_utils::RunMethodUnsafe(
        il2cpp_utils::GetClassFromName("GlobalNamespace", "IPlatformUserModel"),
        "GetUserInfo",
        [callback](GlobalNamespace::UserInfo* userInfo) {
            if (!userInfo) {
                LOG_WARN("Could not get platform UserInfo");
                callback(false, "");
                return;
            }

            std::string userId = to_utf8(csstrtostr(userInfo->platformUserId));
            LOG_INFO("Platform user id: {}", userId);

            // Build request body
            rapidjson::StringBuffer sb;
            rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
            writer.StartObject();
            writer.Key("platform");   writer.String("oculus");
            writer.Key("ticket");     writer.String(userId.c_str()); // ticket = userId for ingame auth
            writer.EndObject();

            std::string postBody = sb.GetString();
            std::string authUrl  = std::string(BASE_URL) + "/v1/auth/ingame";

            HttpPost(authUrl, postBody, [callback, userId](bool ok, std::string resp) {
                if (!ok || resp.empty()) {
                    // Auth failed – store user ID anyway for public API calls
                    AccSaberAPI::loggedInUserId = userId;
                    callback(false, userId);
                    return;
                }

                rapidjson::Document doc;
                doc.Parse(resp.c_str());
                if (doc.HasParseError() || !doc.IsObject()) {
                    callback(false, userId);
                    return;
                }

                // Response: { "token": "...", "userId": "..." }
                if (doc.HasMember("token") && doc["token"].IsString()) {
                    AccSaberAPI::authToken      = doc["token"].GetString();
                    AccSaberAPI::loggedInUserId = SafeGetString(doc, "userId", userId);
                    callback(true, AccSaberAPI::loggedInUserId);
                } else {
                    AccSaberAPI::loggedInUserId = userId;
                    callback(false, userId);
                }
            });
        }
    );
}

// ─── Player Info ──────────────────────────────────────────────────────────

void AccSaberAPI::GetPlayerInfo(
    const std::string& userId,
    std::function<void(std::optional<AccSaberPlayerInfo>)> callback
) {
    std::string url = std::string(BASE_URL) + "/v1/users/" + userId;
    HttpGet(url, true, [callback](bool ok, std::string body) {
        if (!ok || body.empty()) { callback(std::nullopt); return; }

        rapidjson::Document doc;
        doc.Parse(body.c_str());
        if (doc.HasParseError() || !doc.IsObject()) { callback(std::nullopt); return; }

        AccSaberPlayerInfo info;
        info.userId    = SafeGetString(doc, "id");
        info.name      = SafeGetString(doc, "name");
        info.ap        = SafeGetFloat(doc, "ap");
        info.rank      = SafeGetInt(doc, "rank");
        info.level     = SafeGetInt(doc, "level");
        info.xp        = SafeGetFloat(doc, "xp");
        info.avatarUrl = SafeGetString(doc, "avatarUrl");

        callback(info);
    });
}

void AccSaberAPI::GetMyInfo(
    std::function<void(std::optional<AccSaberPlayerInfo>)> callback
) {
    if (loggedInUserId.empty()) { callback(std::nullopt); return; }
    GetPlayerInfo(loggedInUserId, callback);
}

// ─── Map lookup ───────────────────────────────────────────────────────────

void AccSaberAPI::GetMapByHash(
    const std::string& songHash,
    std::function<void(bool, std::string, std::string)> callback
) {
    std::string url = std::string(BASE_URL) + "/v1/maps/hash/" + songHash;
    HttpGet(url, false, [callback](bool ok, std::string body) {
        if (!ok || body.empty()) { callback(false, "", ""); return; }

        rapidjson::Document doc;
        doc.Parse(body.c_str());
        if (doc.HasParseError() || !doc.IsObject()) { callback(false, "", ""); return; }

        // Response includes difficulties array
        if (!doc.HasMember("difficulties") || !doc["difficulties"].IsArray()
            || doc["difficulties"].Empty()) {
            callback(false, "", "");
            return;
        }

        // Pick the first difficulty that matches the currently-playing one
        auto& diffs = doc["difficulties"];
        std::string difficultyId = SafeGetString(diffs[0], "id");
        std::string categoryId   = SafeGetString(diffs[0], "categoryId");

        callback(true, difficultyId, categoryId);
    });
}

// ─── Leaderboard ──────────────────────────────────────────────────────────

void AccSaberAPI::GetDifficultyLeaderboard(
    const std::string& difficultyId,
    std::function<void(std::vector<AccSaberScoreEntry>)> callback
) {
    std::string url = std::string(BASE_URL) + "/v1/maps/difficulties/" + difficultyId + "/scores?page=0&limit=10";
    HttpGet(url, false, [callback](bool ok, std::string body) {
        if (!ok || body.empty()) { callback({}); return; }

        rapidjson::Document doc;
        doc.Parse(body.c_str());
        if (doc.HasParseError()) { callback({}); return; }

        std::vector<AccSaberScoreEntry> entries;

        // Response may be a plain array or { content: [...] }
        const rapidjson::Value* arr = nullptr;
        if (doc.IsArray()) {
            arr = &doc;
        } else if (doc.IsObject() && doc.HasMember("content") && doc["content"].IsArray()) {
            arr = &doc["content"];
        }

        if (!arr) { callback({}); return; }

        for (auto& entry : arr->GetArray()) {
            AccSaberScoreEntry e;
            e.playerName = SafeGetString(entry, "playerName");
            e.playerId   = SafeGetString(entry, "playerId");
            e.ap         = SafeGetFloat(entry, "ap");
            e.accuracy   = SafeGetFloat(entry, "accuracy");
            e.rank       = SafeGetInt(entry, "rank");
            e.rawScore   = SafeGetInt(entry, "score");
            entries.push_back(e);
        }
        callback(entries);
    });
}

// ─── My score for a map ───────────────────────────────────────────────────

void AccSaberAPI::GetMyScoreForMap(
    const std::string& songHash,
    std::function<void(std::optional<AccSaberMapScore>)> callback
) {
    if (loggedInUserId.empty()) { callback(std::nullopt); return; }

    std::string url = std::string(BASE_URL) + "/v1/users/" + loggedInUserId
                      + "/scores/by-hash/" + songHash;
    HttpGet(url, true, [callback](bool ok, std::string body) {
        if (!ok || body.empty()) { callback(std::nullopt); return; }

        rapidjson::Document doc;
        doc.Parse(body.c_str());
        if (doc.HasParseError() || !doc.IsObject()) { callback(std::nullopt); return; }

        AccSaberMapScore score;
        score.ap        = SafeGetFloat(doc, "ap");
        score.accuracy  = SafeGetFloat(doc, "accuracy");
        score.rank      = SafeGetInt(doc, "rank");
        score.rawScore  = SafeGetInt(doc, "score");
        score.ranked    = true;

        callback(score);
    });
}
