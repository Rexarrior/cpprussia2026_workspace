# User-to-User Direct Messaging Plan

## Overview
Add ability to search for users by login/name and send direct messages (DMs) between users, not just in shared channels.

## API Changes Required

### 1. User Search API (Auth Service)
**Endpoint:** `POST /v1/user/search`

Search for users by login or display name prefix.

**Request:**
```json
{
  "query": "jo",
  "limit": 10
}
```

**Response:**
```json
{
  "users": [
    {"login": "john_doe", "name": "John Doe"},
    {"login": "joanna", "name": "Joanna Smith"}
  ]
}
```

### 2. Direct Message Channels (Messaging Service)
**New endpoint:** `POST /v1/direct/get-or-create`

Get or create a direct message channel between current user and another user.

**Request:**
```json
{
  "current_user": {"login": "alice", "name": "Alice", "token": "..."},
  "other_user_login": "bob"
}
```

**Response:**
```json
{
  "channel_id": "dm_alice_bob",
  "other_user": {"login": "bob", "name": "Bob"}
}
```

### 3. List Direct Chats (Messaging Service)
**New endpoint:** `POST /v1/direct/list`

Get all direct message channels for current user.

**Request:**
```json
{
  "current_user": {"login": "alice", "name": "Alice", "token": "..."}
}
```

**Response:**
```json
{
  "direct_chats": [
    {"channel_id": "dm_alice_bob", "other_user": {"login": "bob", "name": "Bob"}, "last_message": {...}},
    {"channel_id": "dm_alice_charlie", "other_user": {"login": "charlie", "name": "Charlie"}, "last_message": {...}}
  ]
}
```

## Data Model Changes

### Messaging Service
Add `direct_channels` storage:
- `direct_channels: {channel_id: {user1, user2, created_at}}`
- Channel ID format: `dm_{sorted_user_logins.join("_")}` e.g., `dm_alice_bob`

## Frontend Changes

### ChatList Component
- Show both channel chats (General, Random) AND direct message chats
- Add "New Direct Message" button that opens user search modal
- Direct chats show other user's avatar and name

### User Search Modal
- Input field for search query
- List of matching users with "Start Chat" button
- On selection, create/get direct channel and navigate to it

### MessengerLayout
- Pass `chat.type` (channel/direct) to MessageWindow
- For direct chats, show "Chatting with {other_user_name}" in header

## Implementation Order

1. Add `POST /v1/user/search` to auth-service
2. Add `POST /v1/direct/get-or-create` to messaging-service
3. Add `POST /v1/direct/list` to messaging-service
4. Update frontend: add user search, direct chat handling
5. Test end-to-end flow

## Files to Modify

### Backend
- `services/auth-service/app/router.py` - add search endpoint
- `services/auth-service/app/schemas.py` - add request/response schemas
- `services/messaging-service/app/router.py` - add direct message endpoints
- `services/messaging-service/app/models.py` - add direct_channels storage

### Frontend
- `src/stores/chats.js` - add direct chat support
- `src/components/messenger/ChatList.vue` - add search modal
- `src/api/index.js` - add user search API, direct message APIs

## Open Questions

1. Should direct message channel IDs be deterministic or generated?
   - **Decision:** Deterministic using sorted user logins, so alice+bob always = dm_alice_bob

2. Should we track "last message" preview in direct chat list?
   - **Decision:** Yes, include last message preview

3. What happens if user searches for themselves?
   - **Decision:** Exclude current user from search results
