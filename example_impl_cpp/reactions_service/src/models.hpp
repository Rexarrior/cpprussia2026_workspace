#pragma once

#include <string>
#include <unordered_map>
#include <mutex>
#include <optional>
#include <random>
#include <vector>
#include <algorithm>
#include <fmt/format.h>

namespace reactions_service {

struct Reaction {
    std::string id;
    int channel_id;
    int message_id;
    std::string user_login;
    std::string animation;
    std::string idempotency_token;
};

class ReactionStorage {
public:
    static ReactionStorage& GetInstance() {
        static ReactionStorage instance;
        return instance;
    }

    // Store or update a reaction
    // Returns true if added, false if removed (or replaced)
    std::pair<bool, std::optional<Reaction>> UpsertReaction(const Reaction& reaction) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string key = MakeKey(reaction.channel_id, reaction.message_id, reaction.user_login);
        auto it = reactions_db_.find(key);
        if (it != reactions_db_.end()) {
            const Reaction& existing = it->second;
            if (existing.idempotency_token == reaction.idempotency_token) {
                // Same idempotency token: return existing
                return {false, existing};
            } else {
                // Different token: remove old, add new
                reactions_db_.erase(it);
            }
        }
        reactions_db_[key] = reaction;
        return {true, reaction};
    }

    // Retrieve reaction by key
    std::optional<Reaction> GetReaction(int channel_id, int message_id, const std::string& user_login) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string key = MakeKey(channel_id, message_id, user_login);
        auto it = reactions_db_.find(key);
        if (it == reactions_db_.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    // Delete reaction by key
    bool DeleteReaction(int channel_id, int message_id, const std::string& user_login) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string key = MakeKey(channel_id, message_id, user_login);
        auto it = reactions_db_.find(key);
        if (it == reactions_db_.end()) {
            return false;
        }
        reactions_db_.erase(it);
        return true;
    }

    // Get all reactions for a given channel_id and message_id
    std::vector<Reaction> GetReactionsForMessage(int channel_id, int message_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<Reaction> result;
        for (const auto& [key, reaction] : reactions_db_) {
            if (reaction.channel_id == channel_id && reaction.message_id == message_id) {
                result.push_back(reaction);
            }
        }
        return result;
    }

    // Generate a unique ID (simple random)
    static std::string GenerateId() {
        static std::random_device rd;
        static std::mt19937_64 gen(rd());
        static std::uniform_int_distribution<uint64_t> dis;
        uint64_t part1 = dis(gen);
        uint64_t part2 = dis(gen);
        char buf[33];
        snprintf(buf, sizeof(buf), "%016lx%016lx", part1, part2);
        return std::string(buf);
    }

private:
    ReactionStorage() = default;
    
    static std::string MakeKey(int channel_id, int message_id, const std::string& user_login) {
        return fmt::format("{}:{}:{}", channel_id, message_id, user_login);
    }

    std::unordered_map<std::string, Reaction> reactions_db_;
    std::mutex mutex_;
};

} // namespace reactions_service