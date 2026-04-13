# Audit Report: User-to-User Direct Messaging

## Implementation Status

### ✅ Completed

#### 1. API Changes (min_messenger.yaml)
- Added `POST /v1/user/search` endpoint schema
- Added `POST /v1/direct/get-or-create` endpoint schema
- Added `POST /v1/direct/list` endpoint schema
- Added `V1PublicUser`, `V1DirectChatInfo` schemas

#### 2. Auth Service - User Search
- Added `V1UserSearchRequest` and `V1UserSearchResponse` schemas
- Implemented `POST /v1/user/search` endpoint
- Searches users by login or name prefix (case-insensitive)
- Returns up to `limit` users (default 10)

#### 3. Messaging Service - Direct Messages
- Added `DirectChannel` model with string channel_id format `dm_{user1}_{user2}`
- Added `direct_channels_db` storage
- Implemented `get_direct_channel_id()` for deterministic channel IDs
- Updated message schema to support both int and string channel_ids
- Implemented `POST /v1/direct/get-or-create` - creates or returns existing DM channel
- Implemented `POST /v1/direct/list` - lists all DM channels for current user

#### 4. Frontend Changes
- Updated API client with `authApi.searchUsers()` and `messagingApi.getOrCreateDirectChat()`, `listDirectChats()`
- Updated chats store with:
  - `channelChats` (predefined: General, Random)
  - `directChats` (fetched from API)
  - `fetchDirectChats()` and `getOrCreateDirectChat()` methods
- Created `UserSearchModal.vue` component:
  - Search input with debounce
  - User list display
  - "Chat" button to start DM
- Updated `ChatList.vue`:
  - Added ➕ button to open user search modal
  - Separated Channels and Direct Messages sections
  - Direct chats fetched on mount

### ⚠️ Known Issues / Limitations

1. **Direct message other_user.name** - Currently returns just the login, not the actual display name (would need to call auth-service to get user info)

2. **No real-time updates** - Direct chat list doesn't auto-refresh when new DM is created

3. **No message history for DMs on fresh load** - Need to fetch messages for direct channels

4. **Browser testing not completed** - Playwright browser had issues, manual testing needed

## Files Modified

### Backend
- `services/auth-service/app/schemas.py` - Added search schemas
- `services/auth-service/app/router.py` - Added search endpoint
- `services/messaging-service/app/schemas.py` - Added direct message schemas
- `services/messaging-service/app/models.py` - Added DirectChannel model
- `services/messaging-service/app/router.py` - Added direct message endpoints
- `services/messaging-service/app/main.py` - Registered direct_router

### Frontend
- `frontend/src/api/index.js` - Added search and direct message APIs
- `frontend/src/stores/chats.js` - Added direct chat support
- `frontend/src/components/messenger/UserSearchModal.vue` - New component
- `frontend/src/components/messenger/ChatList.vue` - Updated with DM support

## Next Steps (if needed)
1. Manual testing of user search and DM creation
2. Fix other_user display name (call auth-service or store name on DM creation)
3. Add polling for direct chat list updates
4. Add "last message" preview in direct chat list
