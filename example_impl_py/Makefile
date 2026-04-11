.PHONY: build up down test test-e2e

build:
	docker-compose build

up:
	docker-compose up -d

down:
	docker-compose down

test:
	cd services/auth-service && pytest
	cd services/messaging-service && pytest
	cd services/notifications-service && pytest
	cd services/files-service && pytest
	cd services/reactions-service && pytest
	cd services/status-service && pytest

test-e2e:
	cd frontend && npx playwright test
