## Re-Audit Report: messaging-service

### Status: FIXED

### Verified Fixes

1. **Added paths section with HTTP methods** - FIXED
   - `paths` section exists in YAML (lines 201-322)
   - Two endpoints defined: `POST /v1/channel/message/new` and `POST /v1/channel/message/by-timestamp`
   - Both endpoints have proper `post` method definitions with `summary`, `requestBody`, and `responses`

2. **Added HTTP status codes** - FIXED
   - All error codes present for both endpoints: 200, 400, 401, 404, 500
   - Each response includes `description` and `content/application/json` with schema reference
   - V1Error schema properly defined with `error` (string) and `code` (int32)

3. **timestamp has format: date-time** - FIXED
   - `V1ChannelMessage.timestamp`: `type: string, format: date-time` (line 75)
   - `V1ChannelMessageByTimestampRequest.from`: `type: string, format: date-time` (line 101)
   - `V1ChannelMessageByTimestampRequest.to`: `type: string, format: date-time` (line 105)
   - Markdown confirms: "Все поля timestamp используют формат ISO8601 (format: date-time)" (line 34)

4. **Added pagination (next_cursor, has_more, limit)** - FIXED
   - `V1ChannelMessageByTimestampResponse` includes:
     - `next_cursor`: `type: string, nullable: true` (lines 130-133)
     - `has_more`: `type: boolean` (lines 134-136)
   - `V1ChannelMessageByTimestampRequest` includes:
     - `limit`: `type: integer, format: int32, minimum: 1, maximum: 1000, default: 100` (lines 107-113)
   - Markdown documents pagination in section "Пагинация" (lines 25-30)

5. **author/current_user unified** - FIXED
   - Markdown now correctly states: "V1ChannelMessage — структура сообщения (id, timestamp, message, current_user)" (line 22)
   - YAML consistently uses `current_user` field throughout (lines 63, 68-69, 81-84, etc.)

6. **Added examples** - FIXED
   - Schema examples present for all major types:
     - `V1CurrentUser` example (lines 54-57)
     - `V1ChannelMessage` example (lines 80-87)
     - `V1ChannelMessageByTimestampRequest` example (lines 114-118)
     - `V1ChannelMessageByTimestampResponse` example (lines 137-147)
     - `V1ChannelMessageNewRequest` example (lines 164-170)
     - `V1ChannelMessageNewResponse` example (lines 180-181)
     - `V1Error` example (lines 197-199)
   - Path-level examples in requestBody and responses for both endpoints

### Remaining Issues

1. **Minor: V1CurrentUser token length constraint** - Remains
   - `token` field has `minLength: 128, maxLength: 128` (lines 46-47)
   - If tokens are JWT, they vary in length; fixed 128-char constraint may cause validation issues
   - **Severity: minor** (was minor before)

2. **Minor: additionalProperties: false consistency** - Remains
   - Most schemas have `additionalProperties: false` (lines 39, 61, 91, etc.)
   - Prevents extensibility; typical OpenAPI practice is opt-in
   - **Severity: minor** (was minor before)

### Summary

All critical and major issues from the original audit have been resolved. The OpenAPI specification is now complete with proper paths, HTTP status codes, timestamp formatting, pagination, field consistency, and examples. The remaining issues are minor and do not affect API functionality or spec completeness.
