# Multi-stage Dockerfile for yolo-timeline HA add-on
# Used by GitHub Actions to build and push to ghcr.io
# Build context must be the repo root (docker build -f yolo-timeline/Dockerfile .)

# ── Stage 1: Build Angular frontend ─────────────────────────────────────────
FROM node:22-slim AS frontend
WORKDIR /frontend
COPY frontend/package*.json ./
RUN npm ci
COPY frontend/ .
RUN npx ng build --base-href /

# ── Stage 2: Build C++ yolo-timeline service ────────────────────────────────
FROM debian:trixie-slim AS cpp-build
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake ninja-build git pkg-config ca-certificates \
    libdrogon-dev libtrantor-dev \
    libpqxx-dev libpq-dev \
    libyaml-cpp-dev libjsoncpp-dev \
    libspdlog-dev libfmt-dev \
    nlohmann-json3-dev \
    uuid-dev libbrotli-dev libsqlite3-dev \
    libhiredis-dev default-libmysqlclient-dev \
    libssl-dev libkrb5-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY yolo_detection_cpp/ ./yolo_detection_cpp/
RUN cmake -S yolo_detection_cpp -B yolo_detection_cpp/build \
        -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_TESTS=OFF \
    && cmake --build yolo_detection_cpp/build --target yolo_timeline

# ── Stage 3: Runtime image ──────────────────────────────────────────────────
FROM debian:trixie-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
    libdrogon1t64 libtrantor1 \
    libpqxx-7.10 libpq5 \
    libyaml-cpp0.8 libjsoncpp26 \
    libspdlog1.15 libfmt10 \
    libuuid1 libbrotli1 libsqlite3-0 libhiredis1.1.0 libmariadb3 \
    libssl3 libkrb5-3 \
    jq curl \
    && rm -rf /var/lib/apt/lists/*

COPY --from=cpp-build \
    /src/yolo_detection_cpp/build/services/timeline/yolo_timeline \
    /usr/local/bin/yolo_timeline
COPY --from=frontend /frontend/dist/browser/ /app/static/

RUN chmod +x /usr/local/bin/yolo_timeline \
    && cp /app/static/index.html /app/static/index.html.template

COPY yolo-timeline/run.sh /run.sh
RUN chmod +x /run.sh

CMD ["/run.sh"]
