# Test Credentials

## API Format

**Register:**
```bash
curl -X POST http://localhost/api/auth/v1/user/registration \
  -H "Content-Type: application/json" \
  -d '{"login":"testuser","password":"testpass","name":"Test User","email":"test@test.com","phone":"123456"}'
```

**Login:**
```bash
curl -X POST http://localhost/api/auth/v1/user/authorization \
  -H "Content-Type: application/json" \
  -d '{"login":"testuser","password":"testpass"}'
```

## Current Test User

| Login     | Password | Token |
|-----------|----------|-------|
| testuser  | testpass | ...   |

## Note

- Data is stored **in-memory** - lost on `docker-compose down` or rebuild
- All 6 microservices store data in memory
- API prefix: `/api/{service}/v1/...`
