# C++ Messenger Services Docker Setup

This directory contains Docker configuration for six C++ microservices built with userver framework.

## Services

| Service | Port | Description | Container Name |
|---------|------|-------------|----------------|
| auth_service | 8001 | User authentication and registration | `example_impl_cpp_auth-service_1` |
| messaging_service | 8002 | Channel and direct messaging | `example_impl_cpp_messaging-service_1` |
| notifications_service | 8003 | Channel notifications | `example_impl_cpp_notifications-service_1` |
| files_service | 8004 | File upload and download | `example_impl_cpp_files-service_1` |
| reactions_service | 8005 | Message reactions (likes) | `example_impl_cpp_reactions-service_1` |
| status_service | 8006 | User status and presence | `example_impl_cpp_status-service_1` |

## Prerequisites

- Docker Engine 20.10+ or Docker Desktop
- docker-compose v2.0+
- sudo privileges (or user in docker group)

## Quick Start

```bash
cd /home/rexarrior/cpprussia2026_workspace/example_impl_cpp

# Build and start all services
sudo docker-compose up -d --build

# Verify all services are running
sudo docker-compose ps

# Test authentication service
curl -X POST http://localhost:8001/v1/user/registration \
  -H "Content-Type: application/json" \
  -d '{"login":"testuser","password":"secret","name":"Test User","email":"test@example.com","phone":"+1234567890"}'

# Stop all services
sudo docker-compose down
```

## Building and Running

### 1. **Build all services**:
   ```bash
   sudo docker-compose build
   ```

### 2. **Start all services**:
   ```bash
   sudo docker-compose up -d
   ```

### 3. **Check status**:
   ```bash
   sudo docker-compose ps
   ```

### 4. **View logs** (real-time):
   ```bash
   # All services
   sudo docker-compose logs -f
   
   # Specific service
   sudo docker-compose logs -f auth-service
   sudo docker-compose logs -f messaging-service
   ```

### 5. **Stop services**:
   ```bash
   sudo docker-compose down
   
   # Remove volumes too
   sudo docker-compose down -v
   ```

## Building a Single Service

```bash
sudo docker-compose build auth-service
sudo docker-compose build messaging-service
# etc.
```

## Testing API Endpoints

Each service exposes HTTP endpoints on its respective port:

### Authentication Service (8001)
```bash
# User registration (returns 128-char token)
curl -X POST http://localhost:8001/v1/user/registration \
  -H "Content-Type: application/json" \
  -d '{"login":"testuser","password":"secret","name":"Test User","email":"test@example.com","phone":"+1234567890"}'

# Response includes token for other services:
# {"current_user":{"token":"6f14ea59d4eae2af98ff51f63aff04e8beca83bf25be44529bf0225dcd4e4ab863bb016e10fd91e29e8c1805b98eeec7263f1e3c1fe4f366f7a54192da02a815","login":"testuser","name":"Test User"}}
```

### Messaging Service (8002)
```bash
# Create direct chat channel (requires token)
curl -X POST http://localhost:8002/v1/direct/get-or-create \
  -H "Authorization: <128-char-token>" \
  -H "Content-Type: application/json" \
  -d '{"current_user":{"token":"<token>","login":"testuser","name":"Test User"},"other_user_login":"otheruser"}'

# List direct messages
curl -X POST http://localhost:8002/v1/direct/list \
  -H "Authorization: <128-char-token>" \
  -H "Content-Type: application/json" \
  -d '{"current_user":{"token":"<token>","login":"testuser","name":"Test User"}}'
```

### Notifications Service (8003)
```bash
# List notifications for channel
curl -X POST http://localhost:8003/v1/channel/notification/list \
  -H "Authorization: <128-char-token>" \
  -H "Content-Type: application/json" \
  -d '{"current_user":{"token":"<token>","login":"testuser","name":"Test User"},"channel_id":"dm_otheruser_testuser"}'
```

### Other Services
```bash
# Files service (8004) - file operations
curl -X POST http://localhost:8004/v1/file/upload \
  -H "Authorization: <128-char-token>" \
  -F "file=@/path/to/file.txt"

# Reactions service (8005) - like/unlike
curl -X POST http://localhost:8005/v1/like/add \
  -H "Authorization: <128-char-token>" \
  -H "Content-Type: application/json" \
  -d '{"current_user":{"token":"<token>","login":"testuser","name":"Test User"},"message_id":123}'

# Status service (8006) - user presence
curl -X POST http://localhost:8006/v1/user/status/set \
  -H "Authorization: <128-char-token>" \
  -H "Content-Type: application/json" \
  -d '{"current_user":{"token":"<token>","login":"testuser","name":"Test User"},"status":"online"}'
```

## Configuration

Each service has its own `static_config.yaml` in its directory:
- `auth_service/static_config.yaml` - port 8001
- `messaging_service/static_config.yaml` - port 8002
- `notifications_service/static_config.yaml` - port 8003
- `files_service/static_config.yaml` - port 8004
- `reactions_service/static_config.yaml` - port 8005
- `status_service/static_config.yaml` - port 8006

Configuration includes:
- Server port binding
- Task processor settings (worker threads)
- Logging configuration (TSKV format to stderr)
- Dynamic config (disabled for standalone operation)
- Coroutine pool settings (stack usage monitor disabled for Docker)

## Docker Architecture

### Base Image
- `ghcr.io/userver-framework/ubuntu-24.04-userver-base:latest`
- Contains pre-installed userver framework and dependencies
- Ubuntu 24.04 with optimized C++ runtime

### Build Strategy
**Single-stage runtime-only build** (current):
- Copies pre-built binaries from host `build/` directory
- Copies configuration files
- Minimal overhead, fast builds

**Multi-stage build** (available but not used):
```dockerfile
# Builder stage: compile with system userver
# Runtime stage: copy binary and config
```

### Network Configuration
- All services share `app-network` bridge network
- Services can communicate internally via service names
- External access via published ports 8001-8006

### Volume Mapping
No persistent volumes - all storage is in-memory.

## Development Workflow

### Rebuilding after Code Changes
```bash
# 1. Rebuild locally
cd /home/rexarrior/cpprussia2026_workspace/example_impl_cpp
rm -rf build && mkdir build && cd build
cmake .. && make -j$(nproc)

# 2. Rebuild Docker image
sudo docker-compose build --no-cache auth-service

# 3. Restart service
sudo docker-compose up -d auth-service

# 4. Verify
sudo docker-compose logs -f auth-service
```

### Accessing Container Shell
```bash
sudo docker-compose exec auth-service /bin/bash
sudo docker-compose exec messaging-service /bin/bash
```

### Inspecting Container Filesystem
```bash
# List files in container
sudo docker-compose exec auth-service ls -la /app

# View config file
sudo docker-compose exec auth-service cat /app/static_config.yaml
```

### Monitoring
```bash
# Container resource usage
sudo docker stats

# Process list inside container
sudo docker-compose exec auth-service ps aux

# Network connections
sudo docker-compose exec auth-service netstat -tulpn
```

## Logging and Debugging

### Log Location
- All logs go to stderr in TSKV format
- Available via `docker-compose logs`
- No persistent log files by default

### Common Log Messages
```
# Startup sequence
tskv	timestamp=...	level=INFO	module=ParseManagerConfigAndSetupLogging	text=Parsed configs from file...
tskv	timestamp=...	level=INFO	module=TaskProcessor	text=creating task_processor...
tskv	timestamp=...	level=INFO	module=Manager	text=Starting components manager

# HTTP requests
tskv	timestamp=...	level=INFO	module=handler-auth	text=HTTP request...

# Periodic tasks
tskv	timestamp=...	level=INFO	module=DoStep	text=http_easy_reinit...

# Errors
tskv	timestamp=...	level=ERROR	module=...	text=Error message...
```

### Debug Log Levels
To enable debug logging, edit `static_config.yaml`:
```yaml
logging:
    loggers:
        default:
            level: debug  # debug, info, warning, error
```

## Cleaning Up

```bash
# Stop and remove containers
sudo docker-compose down

# Stop, remove containers and volumes
sudo docker-compose down -v

# Remove all unused Docker resources
sudo docker system prune -a

# Remove specific images
sudo docker rmi example_impl_cpp_auth-service:latest

# Remove all project images
sudo docker images | grep example_impl_cpp | awk '{print $3}' | xargs sudo docker rmi
```

## Performance Considerations

### Resource Limits
- Each container: ~100MB RAM, ~150MB disk
- Worker threads: 4 main + 1 filesystem per service
- No CPU limits by default

### Scaling
```bash
# Scale specific service (example)
sudo docker-compose up -d --scale auth-service=2

# Load balancer required for multiple instances
```

### Optimization Tips
1. Reduce `worker_threads` in `static_config.yaml` for low-traffic deployments
2. Use `--scale` for high-traffic services
3. Consider adding resource limits in `docker-compose.yml`:
   ```yaml
   deploy:
     resources:
       limits:
         cpus: '0.5'
         memory: 200M
   ```

## Troubleshooting

### "Permission denied" for Docker
```bash
# Add user to docker group
sudo usermod -aG docker $USER

# Log out and back in, or:
newgrp docker
```

### "Port is already allocated"
```bash
# Check what's using the port
sudo lsof -i :8001

# Kill process
sudo kill -9 $(sudo lsof -t -i :8001)

# Or stop Python services if running
cd /home/rexarrior/cpprussia2026_workspace/example_impl_py
sudo docker-compose down
```

### "Container exits immediately"
```bash
# Check logs for error
sudo docker-compose logs auth-service

# Common issues:
# 1. Missing binary in build/ directory
# 2. Invalid static_config.yaml
# 3. Missing stack_usage_monitor_enabled: false
```

### "Connection refused" to service
```bash
# Verify container is running
sudo docker-compose ps

# Check container logs
sudo docker-compose logs auth-service

# Test from inside container
sudo docker-compose exec auth-service curl -v http://localhost:8001
```

### "Endpoint not found" (404)
- Verify request path matches handler registration
- Check if endpoint requires POST vs GET
- Validate JSON body structure
- Ensure Authorization header is 128 characters

### High Memory Usage
```bash
# Monitor memory
sudo docker stats

# Reduce worker_threads in static_config.yaml
# main-task-processor: worker_threads: 2  # instead of 4
```

## Integration Notes

1. **API Compatibility**: Full compatibility with Python FastAPI reference implementation
2. **Frontend**: Works with existing Vue.js frontend without modifications
3. **Data Persistence**: In-memory only - data lost on container restart
4. **Authentication**: Token validation stub (checks 128-char length only)
5. **Port Mapping**: Matches original implementation (8001-8006)

## Security Considerations

⚠️ **For development only** ⚠️

1. Authentication uses simple token length check
2. No TLS/HTTPS (HTTP only)
3. No rate limiting
4. No input validation beyond JSON schema
5. No authentication between services

For production, add:
- TLS termination (nginx reverse proxy)
- JWT token validation
- Rate limiting
- Input sanitization
- Service-to-service authentication

## Maintenance

### Updating Base Image
```bash
# Pull latest base image
sudo docker pull ghcr.io/userver-framework/ubuntu-24.04-userver-base:latest

# Rebuild services
sudo docker-compose build --no-cache
```

### Backup and Restore
No persistent data to backup - all storage is in-memory.

### Health Checks
```bash
# Manual health check
curl -f http://localhost:8001 || echo "Service down"

# Add to docker-compose.yml for automatic checks:
healthcheck:
  test: ["CMD", "curl", "-f", "http://localhost:8001"]
  interval: 30s
  timeout: 10s
  retries: 3
```

## Support

For issues:
1. Check `sudo docker-compose logs`
2. Verify port availability
3. Ensure binaries are built locally
4. Review configuration files

Refer to main README.md for development and building instructions.