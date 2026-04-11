# Auth Service

Сервис аутентификации и регистрации пользователей.

## Функциональность

- **Регистрация** — создание нового пользователя с login, password, name, email, phone
- **Авторизация** — вход по login/password, возврат token и данных пользователя

## API

| Метод | Endpoint | Описание |
|-------|----------|----------|
| POST | /v1/user/registration | Регистрация нового пользователя |
| POST | /v1/user/authorization | Авторизация пользователя |

## Схема данных

- **V1Login** — уникальный login пользователя (string, min 3)
- **V1CurrentUser** — данные текущего пользователя (login, name, token)
- **V1UserAuthorizationResponse** — ответ с данными пользователя и token

## Потенциальные проблемы

- Password передаётся в открытом виде (ожидается HTTPS)
- Token имеет фиксированную длину 128 символов
- Нет logout
- Нет refresh token
