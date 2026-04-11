# Changelog

All notable changes to the C++ Russia userver workshop materials.

## [Unreleased]

### Added

#### Project Documentation
- `min_messenger.yaml` - Original OpenAPI schema from colleague
- `docs/api_changes.md` - Detailed description of all API modifications made during review
- `docs/auth-service.md`, `docs/messaging-service.md`, etc. - Per-service documentation
- `docs/auth-service.yaml`, `docs/messaging-service.yaml`, etc. - Per-service OpenAPI specifications

#### Audit Reports
- `docs/tmp/*-audit.md` - Initial audit reports for all 6 services
- `docs/tmp/*-audit-v2.md` - Re-audit reports after fixes

### Changed

#### API Improvements (based on audit)
- Added proper OpenAPI `paths` sections to all service specifications
- Added HTTP status codes (200, 400, 401, 403, 404, 409, 500) to all endpoints
- Added `V1Error` response schemas to all endpoints
- Fixed `other_user_login` to be required in notifications service
- Replaced inappropriate enum value `"shit"` with `"fire"` in reactions service
- Added pagination support (`next_cursor`, `has_more`) to messaging service
- Added `notification_id` to notifications response
- Added `V1LikeTriggerResponse` with `action` and `current_user_reaction`
- Added GET endpoint for retrieving reactions
- Added status types (online, away, busy, offline) and visibility (public, private)
- Added base64 format specification for file content
- Added timestamp format (date-time) specification

---

## [2026-04-12]

### Added

#### Python/FastAPI Implementation (`example_impl_py/`)

##### Infrastructure
- `docker-compose.yml` - Full stack orchestration (7 services)
- `nginx/nginx.conf` - Reverse proxy with /api/{service}/ routing
- `Makefile` - build, up, down, test, test-e2e targets

##### Backend Services (6 microservices)
- `services/auth-service/` - User registration and authorization (port 8001)
  - POST /v1/user/registration
  - POST /v1/user/authorization
  - Token-based authentication (128 char hex tokens)
  - In-memory user storage with password hashing
  
- `services/messaging-service/` - Channels and messages (port 8002)
  - POST /v1/channel/message/new
  - POST /v1/channel/message/by-timestamp (with pagination)
  - Pre-populated channels (general, random)
  
- `services/notifications-service/` - Message notifications (port 8003)
  - POST /v1/channel/notification/new
  - POST /v1/channel/notification/list
  - Read/unread status tracking
  
- `services/files-service/` - File upload/download (port 8004)
  - POST /v1/file/new (base64 content)
  - POST /v1/file/by-uri
  - Owner-only download authorization
  
- `services/reactions-service/` - Message reactions (port 8005)
  - POST /v1/like/trigger (toggle with idempotency token)
  - GET /v1/like/{channel_id}/{message_id}
  - Animations: like, dislike, heart, fire, okay, LOL, smile
  
- `services/status-service/` - User status (port 8006)
  - POST /v1/user/status/update
  - POST /v1/user/status/by-login
  - Status types: online, away, busy, offline
  - Visibility: public, private
  - TTL-based expiration

##### Frontend (Vue.js)
- `frontend/src/views/` - Login, Register, Messages, Notifications, Files, Reactions, Status views
- `frontend/src/api/` - Axios-based API client
- `frontend/src/router.js` - Vue Router configuration
- `frontend/playwright.config.js` - Playwright test configuration
- `frontend/tests/e2e/app.spec.js` - 12 E2E tests

##### Docker Configuration
- Multi-stage build for frontend (Node.js build → Nginx)
- Python 3.11-slim base for backend services
- Shared Docker network for inter-service communication

### Fixed
- Authorization header mismatch (backend expected `token`, frontend sent `Authorization`)
- Missing `V1AuthorizedUser` import in auth-service
- Token length increased to 128 characters to match schema validation
- Playwright test syntax (require → import)
- CORS configuration for cross-origin requests

### Known Limitations
- In-memory storage (data lost on container restart)
- No persistent database
- No JWT validation (tokens are simple hex strings)
- No rate limiting
- No refresh token mechanism
- File content transmitted as base64 in JSON (not optimal for large files)
- Workshop scope only - not production-ready

---

## Links

- Original Schema: `min_messenger.yaml`
- Documentation: `docs/`
- Implementation: `example_impl_py/`
- API Changes: `docs/api_changes.md`
