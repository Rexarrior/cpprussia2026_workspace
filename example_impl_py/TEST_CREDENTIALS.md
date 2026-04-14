# Test Credentials

## Test Users (10 users)

| Login | Password |
|-------|----------|
| testuser1 | testpassword1 |
| testuser2 | testpassword2 |
| testuser3 | testpassword3 |
| testuser4 | testpassword4 |
| testuser5 | testpassword5 |
| testuser6 | testpassword6 |
| testuser7 | testpassword7 |
| testuser8 | testpassword8 |
| testuser9 | testpassword9 |
| testuser10 | testpassword10 |

## API Examples

**Register:**
```bash
curl -X POST http://localhost/api/auth/v1/user/registration \
  -H "Content-Type: application/json" \
  -d '{"login":"newuser","password":"password123","name":"New User","email":"new@test.com","phone":"123456"}'
```

**Login:**
```bash
curl -X POST http://localhost/api/auth/v1/user/authorization \
  -H "Content-Type: application/json" \
  -d '{"login":"testuser1","password":"testpassword1"}'
```

**Search Users:**
```bash
curl -X POST http://localhost/api/auth/v1/user/search \
  -H "Content-Type: application/json" \
  -d '{"query":"testuser","limit":10}'
```

**Get or Create Direct Chat:**
```bash
curl -X POST http://localhost/api/messaging/v1/direct/get-or-create \
  -H "Content-Type: application/json" \
  -H "Authorization: <token>" \
  -d '{"current_user":{"login":"testuser1","name":"Test User 1","token":"<token>"},"other_user_login":"testuser2"}'
```

## Note

- Data is stored **in-memory** - lost on `docker-compose down` or rebuild
- All 6 microservices store data in memory
- API prefix: `/api/{service}/v1/...`
