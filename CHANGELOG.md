# Changelog

## v1.2.2 (2026-03-04)

### Fixed
- **Video MIME type**: Drogon's `newFileResponse()` served `.mp4` files as `application/octet-stream` — browsers refused to play video. Added `getMimeType()` helper to media controller for correct content-type headers.

### Added
- **Search bar**: text and semantic search with class filter chips (person, dog, car, etc.)
- **Periodic snapshots**: gray dots on timeline for ambient scene snapshots, purple SNAPSHOT badge in viewer
- **Playwright E2E tests**: 26 tests covering recording playback, search, timeline navigation, snapshot display
- **dev-config.yaml**: local development config for running timeline service on NVR
- **Embedding client**: vector embedding support for semantic search
- **Search API endpoints**: `/api/search` with text + semantic modes

### Changed
- HA add-on packaging moved from `yolo-timeline/` to `ha-addon/`
- Proxy config targets port 8080 (timeline service) instead of 8000
- Bumped hms-shared dependency to v1.2.0

### Removed
- Old GitHub Actions workflows, Docker configs, ingress-interceptor.js

## v1.1.1 (2026-02-27)

### Fixed
- Absolute API URLs breaking HA ingress — all Angular URLs now relative

## v1.1.0 (2026-02-27)

### Added
- Camera discovery from DB when config has no cameras section (HA add-on mode)
- HA ingress support with base href injection via supervisor API

## v1.0.0 (2026-02-26)

### Initial release
- C++ timeline service with Drogon HTTP framework
- Angular 19 frontend with camera tabs, date picker, 24-hour timeline
- Event list with recording playback and snapshot display
- Static file serving for events (MP4) and snapshots (JPEG)
- HA add-on packaging with ingress support
