# C++ userver Messenger Services

C++ implementation of the messenger microservices using userver framework.
Port of Python FastAPI services to C++ with identical API for frontend compatibility.

## Services

| Service | Port | Description | API Base Path |
|---------|------|-------------|---------------|
| Auth | 8001 | User registration, authentication, search | `/v1/user/{action}` |
| Messaging | 8002 | Channel messages, direct messages | `/v1/*` (wildcard) |
| Notifications | 8003 | Message notifications | `/v1/channel/notification/{action}` |
| Files | 8004 | File upload/download | `/v1/file/*` (wildcard) |
| Reactions | 8005 | Message reactions (likes) | `/v1/like/*` (wildcard) |
| Status | 8006 | User status (online/away/busy/offline) | `/v1/user/status/*` (wildcard) |

## Quick Start with Docker (Recommended)

```bash
cd /home/rexarrior/cpprussia2026_workspace/example_impl_cpp

# Build all services (optional - binaries are pre-built)
make -j$(nproc)

# Build and start all Docker containers
sudo docker-compose up -d --build

# Check status
sudo docker-compose ps

# Stop all services
sudo docker-compose down
```

## Local Development & Building

### Prerequisites
- C++20 compiler (GCC 13+, Clang 15+)
- CMake 3.14+
- OpenSSL development libraries
- userver framework source code (located at `/home/rexarrior/userver/`)

### Building from Source
```bash
cd /home/rexarrior/cpprussia2026_workspace/example_impl_cpp

# Clean build
rm -rf build && mkdir build && cd build

# Configure with userver from source (default)
cmake ..

# OR configure with system userver (Docker builds)
cmake .. -DUSERVER_FROM_SYSTEM=ON

# Build all services
make -j$(nproc)

# Build specific service
make auth_service
make messaging_service
# etc.
```

### Running Services Locally
```bash
# Run auth service
./build/auth_service/auth_service --config ./auth_service/static_config.yaml

# Run messaging service  
./build/messaging_service/messaging_service --config ./messaging_service/static_config.yaml

# Run notifications service
./build/notifications_service/notifications_service --config ./notifications_service/static_config.yaml

# Run files service
./build/files_service/files_service --config ./files_service/static_config.yaml

# Run reactions service
./build/reactions_service/reactions_service --config ./reactions_service/static_config.yaml

# Run status service
./build/status_service/status_service --config ./status_service/static_config.yaml
```

## Docker Deployment

### Docker Compose Architecture
```yaml
# docker-compose.yml structure:
# - Each service uses common Dockerfile
# - Build arguments: SERVICE and CONFIG_DIR
# - Ports mapped 8001-8006
# - Shared app-network for inter-service communication
```

### Building Docker Images
```bash
# Build specific service
sudo docker-compose build auth-service

# Build all services
sudo docker-compose build

# Force rebuild
sudo docker-compose build --no-cache
```

### Container Management
```bash
# Start all services in background
sudo docker-compose up -d

# Start specific service
sudo docker-compose up -d auth-service

# View logs (all services)
sudo docker-compose logs -f

# View logs for specific service
sudo docker-compose logs -f auth-service
sudo docker-compose logs -f messaging-service

# Check container status
sudo docker-compose ps

# Stop all services
sudo docker-compose down

# Stop and remove volumes
sudo docker-compose down -v
```

## Testing & Debugging

### API Testing with curl
```bash
# Test auth registration
curl -X POST http://localhost:8001/v1/user/registration \
  -H "Content-Type: application/json" \
  -d '{"login":"testuser","password":"secret","name":"Test User","email":"test@example.com","phone":"+1234567890"}'

# Expected response includes 128-character token:
# {"current_user":{"token":"6f14ea59d4eae2af98ff51f63aff04e8beca83bf25be44529bf0225dcd4e4ab863bb016e10fd91e29e8c1805b98eeec7263f1e3c1fe4f366f7a54192da02a815","login":"testuser","name":"Test User"}}

# Test messaging direct channel creation (use token from registration)
curl -X POST http://localhost:8002/v1/direct/get-or-create \
  -H "Authorization: 6f14ea59d4eae2af98ff51f63aff04e8beca83bf25be44529bf0225dcd4e4ab863bb016e10fd91e29e8c1805b98eeec7263f1e3c1fe4f366f7a54192da02a815" \
  -H "Content-Type: application/json" \
  -d '{"current_user":{"token":"6f14ea59d4eae2af98ff51f63aff04e8beca83bf25be44529bf0225dcd4e4ab863bb016e10fd91e29e8c1805b98eeec7263f1e3c1fe4f366f7a54192da02a815","login":"testuser","name":"Test User"},"other_user_login":"otheruser"}'

# Test notifications list
curl -X POST http://localhost:8003/v1/channel/notification/list \
  -H "Authorization: 6f14ea59d4eae2af98ff51f63aff04e8beca83bf25be44529bf0225dcd4e4ab863bb016e10fd91e29e8c1805b98eeec7263f1e3c1fe4f366f7a54192da02a815" \
  -H "Content-Type: application/json" \
  -d '{"current_user":{"token":"6f14ea59d4eae2af98ff51f63aff04e8beca83bf25be44529bf0225dcd4e4ab863bb016e10fd91e29e8c1805b98eeec7263f1e3c1fe4f366f7a54192da02a815","login":"testuser","name":"Test User"},"channel_id":"dm_otheruser_testuser"}'
```

### Logging System

#### Log Format
All services output logs in **TSKV format** (Tab-Separated Key-Value) to stderr by default:
```
tskv	timestamp=2026-05-09T20:34:25.911999	level=INFO	module=ParseManagerConfigAndSetupLogging	text=Parsed configs from file 'static_config.yaml'
```

#### Viewing Logs
```bash
# Docker containers (real-time)
sudo docker-compose logs -f auth-service
sudo docker-compose logs -f messaging-service

# Local execution (redirect to file)
./build/auth_service/auth_service --config ./auth_service/static_config.yaml 2> auth.log

# Filter logs by level
grep "level=ERROR" auth.log
grep "level=WARNING" auth.log
grep "level=INFO" auth.log

# View specific module logs
grep "module=Server" auth.log
grep "module=HttpHandler" auth.log
```

#### Log Levels
- `CRITICAL`: Fatal errors, service will stop
- `ERROR`: Recoverable errors  
- `WARNING`: Warnings, potential issues
- `INFO`: Normal operational messages
- `DEBUG`: Detailed debugging information
- `TRACE`: Very detailed tracing

#### Configuration
Log level and output are configured in `static_config.yaml`:
```yaml
logging:
    loggers:
        default:
            file_path: '@stderr'  # or file path
            level: debug          # debug, info, warning, error
            overflow_behavior: discard
```

### Debugging Common Issues

#### 1. Port Conflicts
```bash
# Check what's using ports 8001-8006
sudo lsof -i :8001-8006

# Kill processes using those ports
sudo kill -9 $(sudo lsof -t -i :8001-8006)

# Alternative: use different ports in static_config.yaml
```

#### 2. Docker Permission Issues
```bash
# Check Docker daemon
sudo docker info

# Fix permission denied
sudo usermod -aG docker $USER
# Then log out and back in
```

#### 3. Service Fails to Start
```bash
# Check container logs
sudo docker-compose logs auth-service

# Common issue: stack_usage_monitor requires userfaultfd
# Solution: Ensure config has:
# coro_pool:
#     stack_usage_monitor_enabled: false
```

#### 4. API Endpoints Return 404
```bash
# Check if service is running
curl -I http://localhost:8001

# Verify handler path in static_config.yaml matches code
# Wildcard paths: /v1/* needs manual path parsing in handler
# Action paths: /v1/user/{action} uses request.GetPathArg("action")
```

#### 5. Memory Issues
```bash
# Monitor container memory
sudo docker stats

# Adjust worker_threads in static_config.yaml
# Default: main-task-processor: worker_threads: 4
```

## Project Structure

```
example_impl_cpp/
├── CMakeLists.txt              # Root CMake with userver options
├── Dockerfile                  # Multi-service Dockerfile
├── docker-compose.yml          # 6-service orchestration
├── .dockerignore              # Build artifacts exclusion
├── README.docker.md           # Docker-specific documentation
├── auth_service/              # Authentication service
│   ├── CMakeLists.txt
│   ├── main.cpp              # Daemon entry point
│   ├── static_config.yaml    # Server config (port 8001)
│   ├── src/
│   │   ├── auth_handler.cpp  # HTTP handler
│   │   ├── models.hpp        # Data structures
│   │   └── schemas.hpp       # JSON (de)serialization
│   └── tests/
├── messaging_service/         # Messaging service (port 8002)
├── notifications_service/     # Notifications service (port 8003)
├── files_service/            # Files service (port 8004)
├── reactions_service/        # Reactions service (port 8005)
└── status_service/           # Status service (port 8006)
```

## Configuration Files

Each service has a `static_config.yaml` with:
- Server port (8001-8006)
- Task processors (main, fs)
- Logging configuration
- Dynamic config (disabled for standalone)
- Handler registration

## Authentication

All services except `/v1/user/registration` require:
- `Authorization` header with 128-character token
- Token validation stub (checks length only)

## Data Persistence

- In-memory storage only (resets on service restart)
- Thread-safe with mutex protection
- No database dependencies

## Performance Considerations

- userver uses asynchronous I/O
- Each service: 4 worker threads (main-task-processor)
- Stack usage monitor disabled for Docker compatibility
- HTTP keep-alive enabled

## Troubleshooting

### "Connection refused" on port 8001
```bash
# Service not running
sudo docker-compose ps
sudo docker-compose up -d auth-service
```

### "Bind for :8001 failed: port is already allocated"
```bash
# Stop Python services if running
cd /home/rexarrior/cpprussia2026_workspace/example_impl_py
sudo docker-compose down

# Or kill local process
sudo kill -9 $(sudo lsof -t -i :8001)
```

### "Failed to initialize StackUsageMonitor"
```bash
# Add to static_config.yaml:
coro_pool:
    stack_usage_monitor_enabled: false
```

### "Endpoint not found" (404)
- Check request path matches handler registration
- POST endpoints require JSON body
- GET endpoints may require query parameters

## Integration with Frontend

The C++ services maintain API compatibility with:
- Original Python FastAPI implementation
- Vue.js frontend (no changes required)
- Same JSON schemas and error formats

## License

Workshop materials for C++ Russia 2026.