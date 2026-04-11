## Audit Report: messaging-service

### Issues Found

#### 1. Critical: OpenAPI Missing Paths (API Completeness)
**Severity: critical**

The YAML file defines only `components/schemas` but does NOT contain any `paths` section. This means:
- No endpoint definitions exist in the OpenAPI spec
- Cannot generate client code from this spec
- Cannot validate API requests/responses

The markdown documents two endpoints:
- `POST /v1/channel/message/new`
- `POST /v1/channel/message/by-timestamp`

But these are never defined in the YAML.

---

#### 2. Critical: Missing Response Schemas
**Severity: critical**

No HTTP response codes are defined anywhere. Missing:
- Success responses (200, 201)
- Error responses (400, 401, 404, 500)
- No error response body schemas

---

#### 3. Major: Schema/Description Mismatch (Author Field)
**Severity: major**

**Markdown states:** `V1ChannelMessage` — структура сообщения (id, timestamp, message, author)

**Actual YAML schema has:** `current_user`, `id`, `timestamp`, `message`

The `author` field is documented but does NOT exist in the schema. Instead, `current_user` is used.

---

#### 4. Major: Timestamp Format Not Specified
**Severity: major**

Multiple timestamp fields are defined as plain `type: string` without `format`:
- `V1ChannelMessage.timestamp`
- `V1ChannelMessageByTimestampRequest.from`
- `V1ChannelMessageByTimestampRequest.to`

Unclear whether:
- ISO8601 format (e.g., `2026-04-11T10:30:00Z`)
- Unix timestamp (e.g., `1712820600`)

Workshop participants will waste time guessing.

---

#### 5. Major: No Pagination
**Severity: major**

`V1ChannelMessageByTimestampResponse.messages` returns an unbounded array. For active channels this can return thousands of messages, causing:
- Memory issues
- Performance degradation
- Network bottlenecks

No pagination parameters (limit, offset, cursor) are defined.

---

#### 6. Minor: Missing Content-Type Specification
**Severity: minor**

No `produces` or `consumes` fields defined at root or path level.

---

#### 7. Minor: No Examples/Placeholders
**Severity: minor**

No `example` fields in schemas. Workshop participants will have no reference for valid request/response payloads.

---

#### 8. Minor: V1CurrentUser Token Length Constraint
**Severity: minor**

`V1CurrentUser.token` has `minLength: 128, maxLength: 128` which is very specific. If tokens are JWT, they usually vary in length. This constraint may cause validation issues.

---

#### 9. Minor: Inconsistent `additionalProperties: false`
**Severity: minor**

Most schemas have `additionalProperties: false` but this is not consistent with typical OpenAPI practice where it's opt-in rather than opt-out. Also, since there are no patterns defined, this effectively prevents extensibility.

---

### Recommendations

1. **Add paths section to YAML:**
   ```yaml
   paths:
     /v1/channel/message/new:
       post:
         summary: Send a new message
         requestBody:
           required: true
           content:
             application/json:
               schema:
                 $ref: '#/components/schemas/V1ChannelMessageNewRequest'
         responses:
           '200':
             description: Success
             content:
               application/json:
                 schema:
                   $ref: '#/components/schemas/V1ChannelMessageNewResponse'
   ```

2. **Define timestamp format explicitly:**
   ```yaml
   timestamp:
     type: string
     format: date-time  # or int64 for unix time
   ```

3. **Add pagination to message list:**
   ```yaml
   V1ChannelMessageByTimestampResponse:
     properties:
       messages:
         type: array
         items:
           $ref: '#/components/schemas/V1ChannelMessage'
       next_cursor:
         type: string
         description: Cursor for next page
       has_more:
         type: boolean
   ```

4. **Fix author field mismatch** — either add `author` to schema or update markdown to reference `current_user`.

5. **Add error response schemas:**
   ```yaml
   V1Error:
     type: object
     required:
       - error
       - code
     properties:
       error:
         type: string
       code:
         type: integer
   ```

6. **Add request/response examples** for all endpoints to facilitate workshop.

7. **Consider making `additionalProperties: false` opt-in** rather than default for flexibility.
