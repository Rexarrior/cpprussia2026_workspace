# AGENTS.md

## Project Overview

C++ Russia userver workshop materials with API schema and Python/FastAPI + Vue.js implementation.

## Key Directories

- `docs/` - OpenAPI specs and per-service documentation (not implementation)
- `example_impl_py/` - Complete running implementation
  - `services/` - 6 FastAPI microservices
  - `frontend/` - Vue.js SPA
  - docker-compose.yml orchestrates all services

## Running the Stack

```bash
cd example_impl_py
docker-compose up -d --build   # Build and start all services
docker-compose down             # Stop all services
```

- App: http://localhost (nginx → frontend:3000)
- API via nginx: http://localhost/api/{service}/
- Direct service ports: auth:8001, messaging:8002, notifications:8003, files:8004, reactions:8005, status:8006

## Common Issues & Fixes

**Docker not running**: If `docker info` fails, start Colima with `colima start`.

**Services won't start**: Kill processes on ports first:
```bash
lsof -ti:8001-8006 | xargs kill -9
```

## Testing

### Backend unit tests
```bash
cd example_impl_py/services/{service}
pip install -r requirements.txt
pytest tests/ -v
```

### Frontend E2E (requires running stack)
```bash
cd example_impl_py/frontend
npm install @playwright/test
npx playwright test tests/e2e/app.spec.js
```

## Backend-First Header Convention

All backend services expect `Authorization` header (not `token`). This was a common bug - verify header names match between frontend axios interceptor and FastAPI `Header(...)` parameters.

## Data Persistence

All services use **in-memory storage**. Data is lost on container restart. For testing, register users via API or curl since registration state doesn't persist across rebuilds.

## API Schema Source

Original schema from colleague: `min_messenger.yaml` (root)

Per-service refined specs: `docs/*-service.yaml`

Implementation: `example_impl_py/services/*/app/router.py`
