## Audit Report: status-service

### Issues Found

#### 1. Incomplete API Definition (Critical)
**Severity:** critical  
**Location:** docs/status-service.yaml  
**Description:** YAML-файл содержит только схемы компонентов (schemas), но не содержит описания paths (endpoints). Отсутствуют:
- `paths` секция с определениями HTTP методов
- Нет явного маппинга между endpoint-ами (`/v1/user/status/update`, `/v1/user/status/by-login`) и схемами запросов/ответов
- Нет описания HTTP кодов ответов (200, 400, 401, 403, 500 и т.д.)
- Нет описания query параметров, заголовков

**Current state:** Задокументированы только схемы данных, но не сами API endpoints.

---

#### 2. V1UserStatusUpdateResponse Empty (Major)
**Severity:** major  
**Location:** docs/status-service.yaml, lines 48-51  
**Description:** 
```yaml
V1UserStatusUpdateResponse:
    type: object
    additionalProperties: false
    properties: {}
```
Ответ на запрос обновления статуса пустой — нет никаких полей. Клиент не получает:
- Подтверждения успешности операции
- Времени истечения статуса (TTL)
- Версии статуса или timestamp создания/обновления

---

#### 3. No Status Schema Validation (Major)
**Severity:** major  
**Location:** docs/status-service.yaml, lines 31-34  
**Description:**
```yaml
V1UserStatus:
    description: Key-values of arbitrary types (in other words, just JSON without a schema)
    type: object
    additionalProperties: true
```
`additionalProperties: true` означает полное отсутствие валидации структуры статуса. Принимается любой произвольный JSON без какой-либо схемы. Это приводит к:
- Невозможности фильтрации или поиска по статус
- Отсутствию гарантий формата данных
- Потенциальным проблемам безопасности при отображении статуса другим пользователям (XSS, injection)

---

#### 4. No Privacy / Authorization (Critical)
**Severity:** critical  
**Location:** docs/status-service.yaml, lines 54-64  
**Description:**
```yaml
V1UserStatusByLoginRequest:
    type: object
    additionalProperties: false
    required:
      - current_user
      - login
    properties:
        current_user:
            $ref: '#/components/schemas/V1CurrentUser'
        login:
            $ref: '#/components/schemas/V1Login'
```
Любой аутентифицированный пользователь может запросить статус любого другого пользователя по логину. Нет механизма:
- Приватности статусов (пользователь не может скрыть свой статус)
- White/black списков пользователей
- Настройки видимости статуса (public/friends/private)

Аналогично, при обновлении статуса (`/v1/user/status/update`) нет проверки, что `current_user` из запроса соответствует аутентифицированному пользователю.

---

#### 5. No TTL or Cleanup Mechanism (Major)
**Severity:** major  
**Location:** docs/status-service.md, lines 25-26  
**Description:** В документации явно указано: "Нет TTL или automatic cleanup". Статусы хранятся бессрочно, что приводит к:
- Неактуальным статусам "пользователь онлайн" и т.п.
- Росту объема хранилища без необходимости
- Невозможности автоматической очистки устаревших данных

Рекомендуется добавить:
- Поле `expires_at` или `ttl_seconds` в V1UserStatusUpdateResponse
- Механизм автоматического удаления/обновления просроченных статусов

---

### Recommendations

1. **Добавить paths секцию в OpenAPI спецификацию:**
   - Определить `POST /v1/user/status/update` с request/response схемами
   - Определить `POST /v1/user/status/by-login` с request/response схемами
   - Указать HTTP коды ответов и возможные ошибки

2. **Расширить V1UserStatusUpdateResponse:**
   ```yaml
   V1UserStatusUpdateResponse:
       type: object
       additionalProperties: false
       required:
         - success
         - updated_at
       properties:
           success:
               type: boolean
           updated_at:
               type: string
               format: date-time
           expires_at:
               type: string
               format: date-time
               description: When the status expires (optional)
   ```

3. **Определить схему валидного статуса:**
   - Если статус — JSON, описать его структуру
   - Если допускается несколько типов статусов, использовать `oneOf`/`anyOf`
   - Пример: `{ "type": "online", "last_activity": "2024-01-01T00:00:00Z" }`

4. **Добавить privacy-контроль:**
   - Поле `visibility` в V1UserStatus: `public`, `friends`, `private`
   - Проверка прав доступа на уровне сервиса
   - Поддержка списка пользователей, которым разрешено видеть статус

5. **Реализовать TTL и cleanup:**
   - Добавить поле `ttl_seconds` или `expires_at` в запрос и ответ
   - Реализовать background job для очистки просроченных статусов
   - Рассмотреть механизм heartbeat для auto-refresh статусов онлайн

6. **Дополнительно рекомендуется:**
   - Добавить endpoint для получения списка всех статусов (с пагинацией)
   - Добавить endpoint для bulk-update статусов
   - Рассмотреть поддержку версионирования статусов
