## Audit Report: files-service

### Issues Found

#### 1. Incomplete OpenAPI Specification (Critical)
**Severity:** critical  
**Description:** YAML файл не содержит секции `paths:`. Описаны только схемы данных в `components/schemas`, но сами endpoint-ы (`POST /v1/file/new`, `POST /v1/file/by-uri`) не определены. Это делает спецификацию непригодной для генерации клиентского кода или валидации запросов.

#### 2. V1FileNewResponse не содержит content (Critical)
**Severity:** critical  
**Description:** При загрузке файла (POST /v1/file/new) пользователь получает только URI файла и информацию о текущем пользователе. Поле `content` отсутствует в ответе. Согласно принципам REST, успешный ответ должен содержать созданный ресурс полностью.

```yaml
V1FileNewResponse:
  properties:
    current_user: V1CurrentUser
    uri: string  # только URI, нет content
```

#### 3. V1FileByUriRequest не описан как endpoint (Major)
**Severity:** major  
**Description:** Схема `V1FileByUriResponse` (линия 79-80) ссылается на `V1File`, но в YAML отсутствует секция `paths`, которая должна связывать запросы и ответы с HTTP методами.

#### 4. Content как string без формата (Major)
**Severity:** major  
**Description:** Поле `content` в V1File определено как простой `type: string` без указания формата:

```yaml
V1File:
  properties:
    content:
      type: string  # нет format: binary или format: base64
```

- В документации (files-service.md) упоминается base64, но в схеме это не отражено
- Для бинарных файлов (изображения, PDF, архивы) это создаёт проблемы:
  - Нет индикации кодирования
  - Увеличение размера на ~33% при base64
  - Неэффективная обработка больших файлов в JSON

#### 5. Отсутствует авторизация на скачивание (Critical)
**Severity:** critical  
**Description:** В V1FileByUriRequest нет механизма авторизации для проверки прав доступа к файлу:

```yaml
V1FileByUriRequest:
  required:
    - current_user
    - uri
  properties:
    current_user: V1CurrentUser  # только идентификация, нет проверки прав
    uri: string
```

- `current_user` позволяет идентифицировать пользователя, но не проверяет, является ли он владельцем файла
- Любой аутентифицированный пользователь может скачать любой файл по URI
- Нет проверки `login` из V1File (владелец файла) против `current_user.login`

#### 6. Отсутствуют метаданные файла (Minor)
**Severity:** minor  
**Description:** V1File содержит только `login`, `filename`, `content`. Отсутствуют:
- `size` — размер файла
- `mime_type` или `content_type` — MIME тип файла
- `created_at` — время создания
- `checksum` — контрольная сумма для проверки целостности

#### 7. URI без схемы (Minor)
**Severity:** minor  
**Description:** URI определён как простая строка без указания схемы доступа:

```yaml
uri:
  type: string
  minLength: 3
```

Неясно, какой протокол используется: `s3://`, `file://`, `https://`, внутренний идентификатор?

---

### Recommendations

1. **Добавить секцию `paths:` в OpenAPI спецификацию:**
   ```yaml
   paths:
     /v1/file/new:
       post:
         requestBody:
           required: true
           content:
             application/json:
               schema:
                 $ref: '#/components/schemas/V1FileNewRequest'
         responses:
           '200':
             description: File uploaded successfully
             content:
               application/json:
                 schema:
                   $ref: '#/components/schemas/V1FileNewResponse'
     /v1/file/by-uri:
       post:
         # аналогично для by-uri
   ```

2. **Добавить content в V1FileNewResponse:**
   ```yaml
   V1FileNewResponse:
     properties:
       current_user:
         $ref: '#/components/schemas/V1CurrentUser'
       uri:
         type: string
       file:
         $ref: '#/components/schemas/V1File'  # полная информация о файле
   ```

3. **Указать формат content:**
   ```yaml
   V1File:
     properties:
       content:
         type: string
         format: base64
         description: Base64 encoded file content
   ```
   Или рассмотреть передачу файла как multipart/form-data.

4. **Добавить проверку авторизации в V1FileByUriRequest:**
   ```yaml
   V1FileByUriRequest:
     properties:
       current_user:
         $ref: '#/components/schemas/V1CurrentUser'
       uri:
         type: string
       # добавить опционально: check_ownership: boolean
   ```
   На бэкенде реализовать проверку: `file.login == current_user.login`.

5. **Расширить V1File метаданными:**
   ```yaml
   V1File:
     properties:
       login: V1Login
       filename: string
       content: string
       size:
         type: integer
         format: int64
       mime_type:
         type: string
         example: image/png
       created_at:
         type: string
         format: date-time
   ```

6. **Уточнить формат URI:**
   ```yaml
   uri:
     type: string
     format: uri
     example: s3://bucket/file-id или https://cdn.example.com/file-id
   ```
