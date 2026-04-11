# API Changes Summary

Document describing all modifications made to the original `min_messenger.yaml` API schema during the C++ Russia userver workshop preparation.

## Why We Changed the API

The original schema provided by the colleague was a **draft** containing only data type definitions (`components/schemas`). It was sufficient for demonstrating compilation but was not a complete API specification. We transformed it into a production-ready OpenAPI 3.0 specification.

---

## Changes by Service

### 1. Auth Service

| Change | Original | After | Reason |
|--------|----------|-------|--------|
| OpenAPI structure | Only `schemas` | Full spec with `openapi`, `info`, `paths` | Required for client code generation and API validation |
| HTTP status codes | None | 200, 400, 401, 409, 500 | API consumers need to know possible error scenarios |
| Token handling | `token` optional in `V1CurrentUser` | Separate `V1AuthorizedUser` with `token` as required | Token should exist only after successful authentication |
| Email validation | `type: string` | `format: email` | Prevent obvious typos, enable client-side validation |
| Password security | Plain text mentioned | Documentation notes HTTPS + backend hashing (bcrypt/argon2) | Security best practice for workshop |

#### Endpoints Added:
- `POST /v1/user/registration` with request/response schemas
- `POST /v1/user/authorization` with request/response schemas

---

### 2. Messaging Service

| Change | Original | After | Reason |
|--------|----------|-------|--------|
| Timestamp format | `type: string` (no format) | `format: date-time` (ISO8601) | Developers must know the format; guessing leads to bugs |
| Pagination | Unbounded array of messages | `next_cursor`, `has_more`, `limit` | Prevent memory/performance issues on active channels |
| Author field | Markdown mentioned `author`, schema had `current_user` | Unified to `current_user` | Consistency between documentation and schema |
| Examples | None | Added request/response examples | Help workshop participants understand valid payloads |

#### Endpoints Added:
- `POST /v1/channel/message/new`
- `POST /v1/channel/message/by-timestamp` with pagination

---

### 3. Notifications Service

| Change | Original | After | Reason |
|--------|----------|-------|--------|
| `other_user_login` | Optional (`required: []`) | **Required** (`required: [other_user_login]`) | Notification without recipient is meaningless |
| Response schema | Empty `{}` | Contains `notification_id` (UUID) | Client needs confirmation that notification was created |
| Read/unread status | Only `message_id[]` returned | `V1NotificationStatus` with `message_id` + `read` flag | Users need to track which notifications they already saw |
| Endpoint paths | Only comments | Full `paths` section with POST methods | Standard OpenAPI requirement |

---

### 4. Files Service

| Change | Original | After | Reason |
|--------|----------|-------|--------|
| `content` format | `type: string` | `format: base64` | Binary files must be base64-encoded in JSON |
| Metadata | Only `login`, `filename`, `content` | Added `size` (int64), `mime_type` (string) | Clients need to display file info before download |
| `V1FileNewResponse` | Only `current_user` + `uri` | Contains nested `file` object with full `V1File` | REST best practice: response should contain created resource |
| URI format | `type: string` | `format: uri` with example `s3://bucket/file-id` | Ambiguous whether s3://, https://, or internal ID |
| Authorization | `current_user` identification only | Documentation notes ownership check (`file.login == current_user.login`) | Prevents unauthorized file access |

#### Endpoints Added:
- `POST /v1/file/new`
- `POST /v1/file/by-uri`

---

### 5. Reactions Service

| Change | Original | After | Reason |
|--------|----------|-------|--------|
| Response schema | **Missing** — only `V1LikeTriggerRequest` existed | Added `V1LikeTriggerResponse` with `action` (added/removed), `current_user_reaction` | Client must know result of toggle operation |
| Enum value `"shit"` | Present | Replaced with `"fire"` | Profanity inappropriate for production service |
| Toggle mechanism | Unclear idempotency_token behavior | Documented: same token returns current state, different token toggles | Developers need clear specification |
| Get reactions | No endpoint to retrieve current reactions | Added `GET /v1/like/{channel_id}/{message_id}` | UI needs to display reaction counts |

#### Endpoints Added:
- `POST /v1/like/trigger` (with response schema)
- `GET /v1/like/{channel_id}/{message_id}`

---

### 6. Status Service

| Change | Original | After | Reason |
|--------|----------|-------|--------|
| `V1UserStatusUpdateResponse` | Empty `{}` | Contains `success` (boolean), `updated_at` (date-time), `expires_at` (date-time, optional) | Client needs confirmation and timing info |
| Status schema | `additionalProperties: true` (any JSON) | Defined `V1StatusType` enum (online, away, busy, offline) + `status_message` string | Validation prevents garbage data; enables filtering |
| Visibility | None | Added `V1Visibility` enum (public, private) | Users should control who sees their status |
| TTL mechanism | No expiration | `expires_at` field available | Prevents stale status data accumulation |

#### Endpoints Added:
- `POST /v1/user/status/update`
- `POST /v1/user/status/by-login`

---

## Common Changes Across All Services

| Change | Reason |
|--------|--------|
| **Added `paths` section** | Original schema was not a valid OpenAPI spec; no client generation possible |
| **Added HTTP status codes** | Original had no error handling documentation |
| **Added `V1Error` schema** | Standardized error response format across all services |
| **Added `examples`** | Workshop participants need reference implementations |
| **Fixed inconsistent indentation/formatting** | YAML must be parseable by standard tools |

---

## What Was NOT Changed

These limitations remain by design (suitable for workshop scope):

- No rate limiting / brute force protection
- No refresh token mechanism
- No device binding / multi-session management
- No multipart/form-data for file uploads (stayed with base64 JSON)
- No WebSocket/subscriptions for real-time updates
- No full-text search
- No user profile management (except status)

---

## Files Modified

| Original | Modified Copy |
|----------|---------------|
| `min_messenger.yaml` | `docs/auth-service.yaml`, `docs/messaging-service.yaml`, etc. |

The original `min_messenger.yaml` in project root was **not modified** — all changes are in `docs/` directory as per-service specifications.
