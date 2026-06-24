#pragma once

#include <string>
#include <functional>
#include <optional>

// ─── Data structures returned by the AccSaber Reloaded API ────────────────

struct AccSaberPlayerInfo {
    std::string userId;
    std::string name;
    float       ap;           // weighted AP
    int         rank;         // global rank
    int         level;        // XP level
    float       xp;
    std::string avatarUrl;
};

struct AccSaberScoreEntry {
    std::string playerName;
    std::string playerId;
    float       ap;
    float       accuracy;     // 0-1
    int         rank;
    int         rawScore;
};

struct AccSaberMapScore {
    float       ap;
    float       accuracy;
    int         rank;         // player's rank on this leaderboard
    int         rawScore;
    bool        ranked;       // is this map ranked on AccSaber?
};

// ─── API client ───────────────────────────────────────────────────────────

class AccSaberAPI {
public:
    static constexpr const char* BASE_URL = "https://api.accsaberreloaded.com";

    // Auth token – populated after a successful ingame auth.
    static std::string authToken;
    static std::string loggedInUserId;

    // Authenticate using the Oculus/Meta platform ticket.
    // Calls POST /v1/auth/ingame with the ticket obtained from the Meta SDK.
    static void AuthenticateIngame(
        std::function<void(bool success, std::string userId)> callback
    );

    // Fetch the profile of any player by their AccSaber/ScoreSaber user ID.
    // GET /v1/users/{userId}
    static void GetPlayerInfo(
        const std::string& userId,
        std::function<void(std::optional<AccSaberPlayerInfo>)> callback
    );

    // Look up a map by its Beat Saber song hash to check if it's ranked.
    // GET /v1/maps/hash/{songHash}
    // Returns the map's first difficultyId, or empty string if not ranked.
    static void GetMapByHash(
        const std::string& songHash,
        std::function<void(bool ranked, std::string difficultyId, std::string categoryId)> callback
    );

    // Get the top scores for a specific difficulty leaderboard.
    // GET /v1/maps/difficulties/{difficultyId}/scores
    static void GetDifficultyLeaderboard(
        const std::string& difficultyId,
        std::function<void(std::vector<AccSaberScoreEntry>)> callback
    );

    // Get the currently logged-in player's score on a specific difficulty.
    // GET /v1/users/{userId}/scores/by-hash/{songHash}
    static void GetMyScoreForMap(
        const std::string& songHash,
        std::function<void(std::optional<AccSaberMapScore>)> callback
    );

    // Get the currently logged-in player's overall info.
    static void GetMyInfo(
        std::function<void(std::optional<AccSaberPlayerInfo>)> callback
    );

private:
    // Internal helper: fire an HTTP GET and call back with the response body.
    static void HttpGet(
        const std::string& url,
        bool authenticated,
        std::function<void(bool success, std::string body)> callback
    );

    static void HttpPost(
        const std::string& url,
        const std::string& body,
        std::function<void(bool success, std::string responseBody)> callback
    );
};
