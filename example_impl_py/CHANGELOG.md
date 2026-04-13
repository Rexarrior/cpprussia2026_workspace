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

#### Telegram-like Messenger UI (2026-04-13)
- Pinia stores: auth, chats, messages, status, reactions
- 8 Vue components: ChatList, ChatListItem, MessageWindow, MessageBubble, MessageInput, StatusIndicator, ReactionBar, MessengerLayout
- Vue Router with lazy loading and navigation guard
- E2E tests for messenger (10 tests, 6 passing)
- Unit tests for stores (Vitest)

#### Messenger Enhancements (2026-04-12)
- **Tailwind CSS v3** - Styled UI components (previously unstyled)
- **Reactions display** - Reactions shown under messages, toggle on hover
- **Reactions fix** - Now wait for real message_id before enabling reactions (temp IDs don't work with backend)
- **File upload** - 📎 attachment button in MessageInput, base64 upload
- **Chat search** - Filter chats by name, "Start chat with X" button
- **Notifications panel** - 🔔 badge with slide-out panel, real data from backend
- **Notifications store** - `src/stores/notifications.js` fetches from notifications API

#### User-to-User Direct Messaging (2026-04-13)
- **User search API** - `POST /v1/user/search` in auth-service
- **Direct messages API** - `POST /v1/direct/get-or-create` and `/v1/direct/list` in messaging-service
- **Direct channel storage** - `dm_{user1}_{user2}` channel ID format
- **UserSearchModal** - New component for finding users and starting DMs
- **Direct chat list** - Shows Channels and Direct Messages sections separately in ChatList

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

### Known Limitations

#### Backend (API constraints)
- **No user search** - No `/v1/user/search` or `/v1/user/find` endpoint exists
- **No direct messages** - API is channel-based only (no user-to-user messaging)
- **In-memory storage** - Data lost on `docker-compose down` or rebuild
- **No JWT validation** - Tokens are simple 128-char hex strings
- **No user profile API** - Cannot fetch user info by login
- **Reactions API limitation** - Backend only tracks reactions count, not which user reacted

#### Frontend
- **Notifications API limited** - Returns only `{message_id, read}`, no rich context (who/why)
- **Direct chat creation** - "Start chat with X" button is UI only, not functional
- **Polling race condition** - Fixed with merge strategy, but can still lose messages under load
- **Browser cache** - Tailwind CSS may need Ctrl+Shift+R after updates

#### Testing
- **Status lookup 404** - Returns 404 for users without status set (expected behavior)

---

## [2026-04-12]

### Added

#### Telegram-like Messenger UI (`frontend/src/`)
- `src/stores/` - Pinia stores for state management
  - `auth.js` - Authentication state
  - `chats.js` - Chat list management
  - `messages.js` - Messages with polling support
  - `status.js` - User status management
  - `reactions.js` - Message reactions
- `src/components/messenger/` - Messenger UI components
  - `ChatList.vue` - Sidebar with chat list
  - `ChatListItem.vue` - Individual chat item
  - `MessageWindow.vue` - Message display area
  - `MessageBubble.vue` - Message bubble component
  - `MessageInput.vue` - Message input textarea
  - `StatusIndicator.vue` - User status dot
  - `ReactionBar.vue` - Hover reaction picker
  - `MessengerLayout.vue` - Main layout (sidebar + messages)
- `src/views/Messenger.vue` - Messenger route view
- `tests/e2e/messenger.spec.js` - Playwright E2E tests
- `tests/unit/stores/` - Vitest unit tests

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

### Known Limitations (General)
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
