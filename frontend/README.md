# YOLO Detection Angular UI

Ring-inspired timeline interface for YOLO detection events with live camera streams.

## Features

- **Live Camera Feeds**: HLS streaming via go2rtc
- **Event Timeline**: 24-hour visual timeline with clickable event markers
- **Event Playback**: View recorded detection events with snapshots
- **AI Context**: Display LLaVA-generated descriptions of detections
- **Mobile-Friendly**: Responsive single-column layout
- **Dark Theme**: Home Assistant-compatible dark UI

## Development

### Prerequisites

- Node.js 18+ and npm
- Running YOLO Detection API at `localhost:8000`
- go2rtc streaming at `192.168.2.5:1984`

### Install Dependencies

```bash
npm install
```

### Development Server

```bash
npm start
```

Navigate to `http://localhost:4200/` or `http://192.168.2.5:4200/` from other devices.

The application will automatically reload if you change any of the source files.

API requests are proxied to `localhost:8000` via `proxy.conf.json`.

### Build for Production

```bash
npm run build:prod
```

Build artifacts will be stored in the `dist/` directory.

## Deployment

### Option 1: nginx (Recommended)

1. Build the production bundle:
   ```bash
   npm run build:prod
   ```

2. Configure nginx:
   ```nginx
   server {
       listen 80;
       server_name 192.168.2.5;

       # Angular app
       location / {
           root /opt/yolo_detection/frontend/dist/browser;
           try_files $uri $uri/ /index.html;
       }

       # API proxy
       location /api/ {
           proxy_pass http://localhost:8000/api/;
       }

       location /events/ {
           proxy_pass http://localhost:8000/events/;
       }

       location /snapshots/ {
           proxy_pass http://localhost:8000/snapshots/;
       }
   }
   ```

3. Restart nginx:
   ```bash
   sudo systemctl restart nginx
   ```

### Option 2: Home Assistant Iframe Panel

Add to Home Assistant `configuration.yaml`:

```yaml
panel_iframe:
  yolo_cameras:
    title: "YOLO Cameras"
    url: "http://192.168.2.5/"
    icon: mdi:cctv
    require_admin: false
```

## Architecture

```
┌─────────────────────────────────────┐
│    Angular Frontend (Port 80)       │
│  Standalone Components + Signals    │
└─────────────┬───────────────────────┘
              │
              ├── HTTP API Calls (/api/)
              ├── Static Assets (/events/, /snapshots/)
              │
              ▼
   ┌─────────────────────┐   ┌────────────────┐
   │  FastAPI (Port 8000) │   │  go2rtc (1984) │
   │  + PostgreSQL        │   │  HLS Streams   │
   └─────────────────────┘   └────────────────┘
```

## Project Structure

```
frontend/
├── src/
│   ├── app/
│   │   ├── core/
│   │   │   ├── services/         # API, Camera, Events, Stream services
│   │   │   └── models/           # TypeScript models
│   │   ├── features/
│   │   │   └── camera-view/      # Main feature with child components
│   │   └── shared/               # Pipes, directives, components
│   ├── environments/
│   └── styles.scss
├── angular.json
├── package.json
├── tailwind.config.js
└── tsconfig.json
```

## API Endpoints Used

- `GET /api/cameras/status` - Camera status with last event time
- `GET /api/events` - List events with filters (camera, date range)
- `GET /api/events/{event_id}` - Event details with all detections
- `GET /api/timeline` - Hourly aggregated event counts
- `GET /events/{filename}` - MP4 recording files
- `GET /snapshots/{filename}` - JPEG snapshot images

## Technologies

- **Angular 17+** - Standalone components with Signals
- **Tailwind CSS** - Utility-first styling
- **Video.js** - HLS live streaming
- **date-fns** - Date formatting
- **Angular CDK** - Virtual scrolling
- **RxJS** - Reactive data streams

## Troubleshooting

### CORS Errors

Ensure FastAPI has CORS middleware configured for your frontend URL:

```python
from fastapi.middleware.cors import CORSMiddleware

app.add_middleware(
    CORSMiddleware,
    allow_origins=["http://192.168.2.5", "http://192.168.2.5:4200"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)
```

### Video Not Playing

- Check go2rtc is running: `docker ps | grep go2rtc`
- Verify HLS stream URL: `http://192.168.2.5:1984/api/stream.m3u8?src=patio`
- Check browser console for Video.js errors

### No Events Showing

- Verify API is accessible: `curl http://localhost:8000/api/events?camera_id=patio`
- Check database connection in API logs: `journalctl -u yolo-detection -f`
- Verify date range is correct

## License

Part of the YOLO Detection Service project.
