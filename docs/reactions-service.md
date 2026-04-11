# Reactions Service

Сервис реакций (лайков/дизлайков) на сообщения.

## Функциональность

- **Toggle реакции** — добавление/удаление анимации на сообщение

## API

| Метод | Endpoint | Описание |
|-------|----------|----------|
| POST | /v1/like/trigger | Добавить/убрать реакцию |

## Схема данных

- **V1LikeTriggerRequest** — current_user, idempotency_token, channel_id, message_id, animation
- **animation** — enum: like, dislike, heart, shit, okay, LOL, smile

## Потенциальные проблемы

- Нет response — непонятно как получить текущие реакции
- idempotency_token для toggle (неясно как работает remove)
- "shit" как название анимации странно для production
- Не возвращает текущее состояние после toggle
