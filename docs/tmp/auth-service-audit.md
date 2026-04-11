## Audit Report: auth-service

### Issues Found

#### Critical

1. **Missing HTTP Status Codes in YAML**
   - В `auth-service.yaml` отсутствуют описания HTTP status codes для ответов
   - Не определено, какие коды возвращаются при ошибках (400, 401, 403, 409, 500)
   - Согласно спецификации OpenAPI, каждая операция должна иметь responses с кодами

2. **Missing Paths/Endpoints Description**
   - YAML файл содержит только схемы (`components/schemas`), но не содержит описания `paths`
   - Нет связи между HTTP методами (POST) и схемами запросов/ответов
   - Отсутствует описание endpoints в формате OpenAPI

#### Major

3. **Token Not Required in V1CurrentUser**
   - В `V1CurrentUser` поле `token` не указано в `required`, хотя логически оно должно быть обязательным после авторизации
   - Это позволяет возвращать пользователя без токена, что не имеет смысла

4. **Password in Plain Text**
   - Пароль передается и хранится в открытом виде
   - Нет хеширования, нет указания на требования к сложности пароля
   - `minLength: 6` — слишком слабое ограничение

5. **Token Fixed Length 128 Characters**
   - Токен имеет фиксированную длину `minLength: 128, maxLength: 128`
   - Для UUID-based токенов это избыточно; для JWT — нетипично
   - Не указан формат токена (JWT, opaque, UUID)

6. **Missing Logout Endpoint**
   - Нет возможности завершить сессию
   - Token остается валидным бесконечно

7. **Missing Refresh Token**
   - При компрометации токена нет механизма его отзыва
   - Пользователь не может "обновить" сессию

8. **V1UserRegistrationResponse → V1UserAuthorizationResponse**
   - `V1UserRegistrationResponse` просто ссылается на `V1UserAuthorizationResponse` через `$ref`
   - При регистрации может потребоваться дополнительная информация (e.g., подтверждение email)
   - Семантически неверно: регистрация и авторизация — разные операции

#### Minor

9. **No Email/Phone Schema Validation**
   - `email` и `phone` определены как `type: string` без формата
   - Нет паттерна валидации (regex для email, формат телефона)
   - `minLength: 3` для phone — явно недостаточно

10. **Inconsistent Language in Documentation**
    - `auth-service.md` содержит смесь русского и английского:
      - "Авторизация", "Регистрация" (русский)
      - "login", "password", "token" (английский)
      - "Authorization scenario", "Registration scenario" (английский в YAML)
    - Нет единого стиля документации

11. **No Rate Limiting / Brute Force Protection**
    - Отсутствуют указания на rate limiting
    - Нет защиты от перебора паролей

12. **No Device Binding or Multiple Sessions**
    - Не описана возможность управления множественными сессиями
    - Не описан механизм привязки устройства

13. **Missing Error Scenarios**
    - Не описаны ошибки: "user already exists" (409?), "invalid credentials" (401?)
    - `V1Error` описан, но не привязан к операциям

14. **Missing Password Requirements**
    - Нет требований к сложности пароля (uppercase, digits, special chars)
    - Только `minLength: 6`

### Recommendations

#### Critical (должны быть исправлены до workshop)

1. **Добавить описание paths в YAML**
   ```yaml
   paths:
     /v1/user/registration:
       post:
         summary: Register new user
         requestBody:
           required: true
           content:
             application/json:
               schema:
                 $ref: '#/components/schemas/V1UserRegistrationRequest'
         responses:
           '200':
             description: Registration successful
             content:
               application/json:
                 schema:
                   $ref: '#/components/schemas/V1UserRegistrationResponse'
   ```

2. **Добавить HTTP Status Codes**
   - 400 — Validation error
   - 401 — Unauthorized (invalid credentials)
   - 409 — Conflict (user already exists)
   - 500 — Internal server error

#### Major (рекомендуется исправить)

3. **Сделать token required в V1CurrentUser или создать отдельную схему для авторизованного пользователя**

4. **Указать hash хеширование паролей** (bcrypt, argon2) или описать требования к передаче по HTTPS

5. **Уточнить формат токена** (JWT с указанием структуры или opaque token)

6. **Добавить endpoints для logout и refresh token**

7. **Разделить V1UserRegistrationResponse и V1UserAuthorizationResponse** — создать отдельные схемы с соответствующими полями

#### Minor (улучшение для workshop)

8. **Добавить валидацию для email** — `format: email` или паттерн regex

9. **Унифицировать язык документации** — выбрать русский или английский

10. **Описать retry/rate limit policy**

11. **Добавить схемы для error responses привязанные к конкретным операциям**

### Summary

| Category | Count |
|----------|-------|
| Critical | 2 |
| Major | 6 |
| Minor | 6 |
| **Total** | **14** |

**Overall Assessment**: API имеет минимальный функционал для демонстрации концепции аутентификации, но для production-ready сервиса или полноценного workshop необходимо устранить минимум 2 critical и 6 major проблем. Особое внимание следует уделить документированию HTTP status codes и формата токена.
