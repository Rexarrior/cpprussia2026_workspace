#include "files_handler.hpp"
#include "schemas.hpp"
#include "models.hpp"

#include <userver/formats/json.hpp>
#include <userver/http/common_headers.hpp>
#include <userver/server/http/http_response.hpp>
#include <fmt/format.h>
#include <stdexcept>

namespace files_service {

FilesHandler::FilesHandler(const components::ComponentConfig& config,
                           const components::ComponentContext& context)
    : HttpHandlerBase(config, context) {
}

std::string FilesHandler::HandleRequest(server::http::HttpRequest& request,
                                        server::request::RequestContext&) const {
    const auto& path = request.GetRequestPath();
    const auto& method = request.GetMethod();

    request.GetHttpResponse().SetContentType(http::content_type::kApplicationJson);

    // Route based on path and method
    if (path == "/v1/file/new" && method == server::http::HttpMethod::kPost) {
        return HandleUpload(request);
    } else if (path == "/v1/file/by-uri" && method == server::http::HttpMethod::kPost) {
        return HandleGetByUri(request);
    }

    // Unknown endpoint
    request.GetHttpResponse().SetStatus(server::http::HttpStatus::kNotFound);
    return ToJson(V1Error{"not_found", "Endpoint not found", std::nullopt});
}

std::string FilesHandler::HandleUpload(const server::http::HttpRequest& request) const {
    try {
        // Check Authorization header
        auto auth_header = request.GetHeader("Authorization");
        if (auth_header.size() != 128) {
            return ToJson(V1Error{"invalid_token", "Invalid token", std::nullopt});
        }

        // Parse request body
        auto body = request.RequestBody();
        V1FileNewRequest req = ParseFileNewRequest(body);

        // Generate file ID and URI
        std::string file_id = FileStorage::GenerateFileId();
        std::string uri = fmt::format("s3://files-bucket/{}", file_id);
        std::size_t size = FileStorage::ComputeSize(req.content);
        std::string mime_type = FileStorage::DetectMimeType(req.filename);

        // Create File object
        File file{
            .file_id = file_id,
            .login = req.login,
            .filename = req.filename,
            .content_base64 = req.content,
            .size = size,
            .mime_type = mime_type,
            .uri = uri
        };

        // Store file
        FileStorage::GetInstance().StoreFile(file);

        // Prepare response
        V1CurrentUser current_user{
            .token = auth_header,
            .login = "unknown",
            .name = "Unknown User"
        };
        V1File file_response{
            .login = req.login,
            .filename = req.filename,
            .content = req.content
        };
        V1FileNewResponse response{
            .current_user = current_user,
            .uri = uri,
            .file = file_response
        };
        return ToJson(response);
    } catch (const std::exception& e) {
        return ToJson(V1Error{"bad_request", e.what(), std::nullopt});
    }
}

std::string FilesHandler::HandleGetByUri(const server::http::HttpRequest& request) const {
    try {
        // Check Authorization header
        auto auth_header = request.GetHeader("Authorization");
        if (auth_header.size() != 128) {
            return ToJson(V1Error{"invalid_token", "Invalid token", std::nullopt});
        }

        // Parse request body
        auto body = request.RequestBody();
        V1FileByUriRequest req = ParseFileByUriRequest(body);

        // Validate token matches current_user token (optional)
        if (req.current_user.token != auth_header) {
            // This might be considered invalid, but Python implementation only checks length.
            // We'll just continue.
        }

        // Retrieve file
        auto opt_file = FileStorage::GetInstance().GetFile(req.uri);
        if (!opt_file.has_value()) {
            return ToJson(V1Error{"file_not_found", "File not found", std::nullopt});
        }
        const File& file = *opt_file;

        // Check authorization: file must belong to the requesting user (login match)
        if (file.login != req.current_user.login) {
            return ToJson(V1Error{"forbidden", "Not authorized to download this file", std::nullopt});
        }

        // Prepare response
        V1FileByUriResponse response{
            .login = file.login,
            .filename = file.filename,
            .content = file.content_base64
        };
        return ToJson(response);
    } catch (const std::exception& e) {
        return ToJson(V1Error{"bad_request", e.what(), std::nullopt});
    }
}

} // namespace files_service
