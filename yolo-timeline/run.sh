#!/usr/bin/env bash
set -euo pipefail

OPTIONS=/data/options.json

# Read HA add-on options via jq
DB_HOST=$(jq -r '.db_host' "$OPTIONS")
DB_PORT=$(jq -r '.db_port' "$OPTIONS")
DB_USER=$(jq -r '.db_user' "$OPTIONS")
DB_PASSWORD=$(jq -r '.db_password' "$OPTIONS")
DB_NAME=$(jq -r '.db_name' "$OPTIONS")
DETECTION_URL=$(jq -r '.detection_service_url' "$OPTIONS")
EVENTS_DIR=$(jq -r '.events_dir' "$OPTIONS")
SNAPSHOTS_DIR=$(jq -r '.snapshots_dir' "$OPTIONS")

echo "[yolo-timeline] Starting up"
echo "[yolo-timeline] DB: ${DB_HOST}:${DB_PORT}/${DB_NAME}"
echo "[yolo-timeline] Detection service: ${DETECTION_URL}"
echo "[yolo-timeline] Events: ${EVENTS_DIR} | Snapshots: ${SNAPSHOTS_DIR}"

# Patch index.html with correct base href for HA ingress
# Query supervisor API for ingress_entry (requires hassio_api: true in config.yaml)
cp /app/static/index.html.template /app/static/index.html
INGRESS_ENTRY=""
if [ -n "${SUPERVISOR_TOKEN:-}" ]; then
    INGRESS_ENTRY=$(curl -s \
        -H "Authorization: Bearer ${SUPERVISOR_TOKEN}" \
        http://supervisor/addons/self/info 2>/dev/null \
        | jq -r '.data.ingress_entry // ""' 2>/dev/null || true)
fi
if [ -n "${INGRESS_ENTRY}" ]; then
    INGRESS_BASE="${INGRESS_ENTRY%/}/"
    sed -i "s|<base href=\"/\">|<base href=\"${INGRESS_BASE}\">|" /app/static/index.html
    echo "[yolo-timeline] Ingress base href: ${INGRESS_BASE}"
else
    echo "[yolo-timeline] No ingress path found, using base href /"
fi

# Generate config.yaml for the C++ service
cat > /app/config.yaml << EOF
database:
  host: "${DB_HOST}"
  port: ${DB_PORT}
  user: "${DB_USER}"
  password: "${DB_PASSWORD}"
  database: "${DB_NAME}"
  pool_size: 4

timeline:
  host: "0.0.0.0"
  port: 8080
  static_files_path: "/app/static"
  events_dir: "${EVENTS_DIR}"
  snapshots_dir: "${SNAPSHOTS_DIR}"
  detection_service_url: "${DETECTION_URL}"
  cors_origins: []

logging:
  level: "INFO"
  file: ""
  max_bytes: 10485760
  backup_count: 5
EOF

exec /usr/local/bin/yolo_timeline --config /app/config.yaml
