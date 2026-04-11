# Status Service

Сервис пользовательских статусов.

## Функциональность

- **Обновление статуса** — установка произвольного JSON-статуса
- **Получение статуса** — получение статуса по login пользователя

## API

| Метод | Endpoint | Описание |
|-------|----------|----------|
| POST | /v1/user/status/update | Обновить свой статус |
| POST | /v1/user/status/by-login | Получить статус пользователя |

## Схема данных

- **V1UserStatus** — произвольный JSON (additionalProperties: true)
- **V1UserStatusUpdateRequest** — current_user, status
- **V1UserStatusByLoginRequest** — current_user, login

## Потенциальные проблемы

- Статус произвольный JSON — нет валидации схемы
- Нет TTL или automatic cleanup
- Нет списка всех статусов
- Кто угодно может читать чужой статус (нет privacy)
