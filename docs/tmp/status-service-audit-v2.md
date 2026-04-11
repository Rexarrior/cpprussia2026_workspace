## Re-Audit Report: status-service

### Status: FIXED

### Verified Fixes

1. **Добавлена секция paths с HTTP методами** - Полностью исправлено
   - Определены два endpoint-а: `POST /v1/user/status/update` и `POST /v1/user/status/by-login`
   - Указаны operationId, summary, requestBody, responses
   - Привязаны схемы запросов и ответов через $ref

2. **V1UserStatusUpdateResponse содержит success, updated_at, expires_at** - Полностью исправлено
   - Добавлено поле `success: boolean` (required)
   - Добавлено поле `updated_at: date-time` (required)
   - Добавлено поле `expires_at: date-time` (optional)
   - Добавлены примеры значений в ответе

3. **Добавлены HTTP status codes** - Полностью исправлено
   - Для `/v1/user/status/update`: 200, 400, 401, 500
   - Для `/v1/user/status/by-login`: 200, 400, 401, 403, 404, 500
   - Каждый код содержит описание и пример response schema

4. **Добавлен V1StatusType enum** - Полностью исправлено
   - enum: online, away, busy, offline
   - Добавлено описание и пример значения
   - Используется в V1UserStatus.status_type

5. **Добавлен visibility enum** - Полностью исправлено
   - enum: public, private
   - Добавлено описание и пример значения
   - Добавлен в V1UserStatus как поле visibility

6. **Добавлены examples** - Полностью исправлено
   - Примеры в request body для обоих endpoints
   - Примеры в response body для обоих endpoints
   - Примеры в схемах V1Login, V1CurrentUser, V1UserStatus

### Remaining Issues

Отсутствуют в текущем scope (не были запрошены для исправления):
- Нет TTL или automatic cleanup механизма (отмечено в docs/status-service.md как known limitation)
- Нет endpoint для получения списка всех статусов

### Conclusion

Все запрошенные исправления выполнены. Спецификация OpenAPI полностью работоспособна и содержит все необходимые компоненты для документирования API status-service.
