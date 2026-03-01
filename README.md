# hms-timeline

[![Buy Me A Coffee](https://img.shields.io/badge/Buy%20Me%20A%20Coffee-support-%23FFDD00.svg?logo=buy-me-a-coffee)](https://www.buymeacoffee.com/aamat09)
[![Build](https://github.com/hms-homelab/hms-timeline/actions/workflows/build-timeline.yml/badge.svg)](https://github.com/hms-homelab/hms-timeline/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

Security camera timeline viewer for Home Assistant. Built with Angular + C++ (Drogon).

## Features

- Live camera snapshots with 2-second refresh
- 24-hour event timeline with hourly heat map
- Detection event list with snapshot thumbnails
- MP4 recording playback
- Home Assistant add-on with ingress support

## Architecture

```
PostgreSQL (events)  +  /mnt/ssd/{events,snapshots}
          │
          ▼
  yolo-timeline (C++ / Drogon)   ← REST API + file serving
          │
          ▼
  Angular UI                     ← camera tabs, timeline, recordings
```

The C++ service reads from the same PostgreSQL database and filesystem
that the detection engine writes to. No coupling between services.

## Docker Quick Start

```bash
docker pull ghcr.io/hms-homelab/yolo-timeline:latest
```

## Home Assistant Add-on

Add this repository to HA:
```
https://github.com/hms-homelab/hms-timeline
```

Then install **YOLO Detection Timeline** from the add-on store.

Configure:
- `db_host` / `db_port` / `db_user` / `db_password` / `db_name` — PostgreSQL connection
- `detection_service_url` — URL of the yolo-detection service (for live snapshots)
- `events_dir` — path to MP4 recordings (e.g. `/media/cctv`)
- `snapshots_dir` — path to JPEG snapshots (e.g. `/media/snapshots`)

## Local Development

```bash
# Run Angular dev server (proxies API to localhost:8080)
cd frontend && npm install && npm start

# Build C++ service
cd yolo_detection_cpp
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF
cmake --build build --target yolo_timeline

# Run with config
./build/services/timeline/yolo_timeline --config config.yaml
```

---

## Support

[![Buy Me A Coffee](https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png)](https://www.buymeacoffee.com/aamat09)
