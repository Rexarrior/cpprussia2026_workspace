## Audit Report: notifications-service

### Issues Found

#### 1. CRITICAL: Empty Response for Notification Creation
- **Severity**: critical
- **Location**: `V1ChannelNotificationNewResponse` (lines 87-90 in YAML)
- **Description**: Response schema is completely empty (`properties: {}`). The API returns an empty JSON object `{}` on successful notification creation with no confirmation, notification ID, or error details.
- **Impact**: Clients cannot verify if notification was created successfully. No mechanism to handle errors or correlate created notifications.

#### 2. CRITICAL: Missing Required Field for Core Functionality
- **Severity**: critical
- **Location**: `V1ChannelNotificationNewRequest` (lines 69-86 in YAML)
- **Description**: `other_user_login` is NOT in the `required` array (line 73-76), making it optional. However, this field is essential for notification creation - without specifying the recipient, the notification purpose is defeated.
- **Impact**: API allows creating notifications without a recipient, which is logically invalid.

#### 3. MAJOR: Incomplete API Specification in YAML
- **Severity**: major
- **Location**: YAML file lacks OpenAPI paths/operations section
- **Description**: The YAML file only contains component schemas but does not define the actual API endpoints (paths, HTTP methods, request/response mappings). This makes the YAML file unusable for OpenAPI validation/generation.
- **Impact**: API specification is incomplete and cannot be used for client generation or validation.

#### 4. MAJOR: No Error Responses Defined
- **Severity**: major
- **Location**: Both endpoints lack error response schemas
- **Description**: No documentation of error cases (400, 401, 403, 404, 500). No `components/responses` section defining error formats.
- **Impact**: Clients have no guidance on error handling.

#### 5. MAJOR: Inconsistent Field Documentation
- **Severity**: major
- **Location**: `V1CurrentUser` (lines 24-40 in YAML)
- **Description**: Schema requires both `login` and `name` (line 28-30), but the markdown documentation (line 19) only mentions `current_user` without specifying its structure. No mention of `name` field requirement.
- **Impact**: Developers may implement incomplete request structures.

#### 6. MINOR: Unclear Notification Scope Semantics
- **Severity**: minor
- **Location**: API design overall
- **Description**: It is unclear whether notifications are global (across all channels) or scoped to a specific channel. The endpoint path includes `/channel/` but notification list does not return channel information, only message IDs.
- **Impact**: API semantics are confusing. Consumers don't understand the scope of notifications.

#### 7. MINOR: No Read/Unread Status Mechanism
- **Severity**: minor
- **Location**: `V1ChannelNotificationListResponse` (lines 55-66 in YAML)
- **Description**: The list endpoint returns only message IDs with no indication of read/unread status.
- **Impact**: No way to track which notifications have been seen by user.

#### 8. MINOR: Inconsistent Type Reference
- **Severity**: minor
- **Location**: `V1ChannelNotificationNewRequest` (line 84-85 in YAML)
- **Description**: `other_user_login` uses `$ref: '#/components/schemas/V1Login'` which is a string type with `minLength: 3`. The reference is correct, but semantically `V1Login` describes "Unique user ID in the system" which may be misleading for a login field.
- **Impact**: Minor semantic confusion.

---

### Recommendations

1. **Empty Response (CRITICAL)**:
   - Add `notification_id` (UUID or integer) to `V1ChannelNotificationNewResponse`
   - Include timestamp of creation
   - Consider adding status field

2. **Missing Required Field (CRITICAL)**:
   - Add `other_user_login` to the `required` array in `V1ChannelNotificationNewRequest`

3. **Incomplete API Specification (MAJOR)**:
   - Add OpenAPI `paths` section defining:
     ```yaml
     paths:
       /v1/channel/notification/new:
         post:
           requestBody:
             content:
               application/json:
                 schema:
                   $ref: '#/components/schemas/V1ChannelNotificationNewRequest'
           responses:
             '200':
               description: Success
               content:
                 application/json:
                   schema:
                     $ref: '#/components/schemas/V1ChannelNotificationNewResponse'
       /v1/channel/notification/list:
         post:
           # similar structure
     ```

4. **Error Responses (MAJOR)**:
   - Add `components/responses` section with standard errors:
     ```yaml
     components:
       responses:
         BadRequest:
           description: Invalid request
         Unauthorized:
           description: Not authorized
         NotFound:
           description: Resource not found
     ```

5. **Documentation (MAJOR)**:
   - Update markdown documentation to describe `V1CurrentUser` structure fully
   - Clarify what fields are required for each operation

6. **Semantics (MINOR)**:
   - Clarify if notifications are per-channel or global
   - Consider renaming endpoint or adding `scope` field
   - If channel-scoped: document this clearly
   - If global: consider removing `/channel/` from path

7. **Read/Unread (MINOR)**:
   - Extend `V1ChannelNotificationListResponse` to include read status:
     ```yaml
     notifications:
       type: array
       items:
         type: object
         properties:
           message_id:
             type: integer
             format: int64
           read:
             type: boolean
     ```
   - Or add separate endpoint for marking as read

8. **Schema Clarity (MINOR)**:
   - Rename `V1Login` to `V1UserLogin` or `V1Username` for clarity
   - Add examples to schemas
