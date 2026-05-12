#pragma once

#include <string>
#include <unordered_map>
#include <mutex>
#include <optional>
#include <cstdint>
#include <random>

namespace files_service {

struct File {
    std::string file_id;
    std::string login;
    std::string filename;
    std::string content_base64;
    std::size_t size;
    std::string mime_type;
    std::string uri;
};

class FileStorage {
public:
    static FileStorage& GetInstance() {
        static FileStorage instance;
        return instance;
    }

    // Store a file, return its URI
    std::string StoreFile(const File& file) {
        std::lock_guard<std::mutex> lock(mutex_);
        files_db_[file.uri] = file;
        return file.uri;
    }

    // Retrieve a file by URI
    std::optional<File> GetFile(const std::string& uri) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = files_db_.find(uri);
        if (it == files_db_.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    // Generate a unique file ID (UUID-like)
    static std::string GenerateFileId() {
        static std::random_device rd;
        static std::mt19937_64 gen(rd());
        static std::uniform_int_distribution<uint64_t> dis;
        uint64_t part1 = dis(gen);
        uint64_t part2 = dis(gen);
        char buf[33];
        snprintf(buf, sizeof(buf), "%016lx%016lx", part1, part2);
        return std::string(buf);
    }

    // Compute size from base64 content
    static std::size_t ComputeSize(const std::string& content_base64) {
        // Basic estimation: each 4 base64 chars encode 3 bytes
        // Remove padding characters '='
        std::size_t len = content_base64.size();
        std::size_t padding = 0;
        if (len > 0 && content_base64[len - 1] == '=') {
            padding++;
            if (len > 1 && content_base64[len - 2] == '=') padding++;
        }
        return (len * 3) / 4 - padding;
    }

    // Detect MIME type from filename extension
    static std::string DetectMimeType(const std::string& filename) {
        std::string ext;
        auto dot_pos = filename.find_last_of('.');
        if (dot_pos != std::string::npos && dot_pos + 1 < filename.size()) {
            ext = filename.substr(dot_pos + 1);
            // Convert to lowercase
            for (char& c : ext) c = std::tolower(static_cast<unsigned char>(c));
        }
        static const std::unordered_map<std::string, std::string> mime_map = {
            {"jpg", "image/jpeg"},
            {"jpeg", "image/jpeg"},
            {"png", "image/png"},
            {"gif", "image/gif"},
            {"pdf", "application/pdf"},
            {"txt", "text/plain"},
            {"json", "application/json"},
            {"html", "text/html"},
            {"css", "text/css"},
            {"js", "application/javascript"},
        };
        auto it = mime_map.find(ext);
        if (it != mime_map.end()) {
            return it->second;
        }
        return "application/octet-stream";
    }

private:
    FileStorage() = default;
    std::unordered_map<std::string, File> files_db_;
    std::mutex mutex_;
};

} // namespace files_service
