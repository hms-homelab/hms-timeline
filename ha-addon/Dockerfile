ARG BUILD_FROM=debian:trixie-slim

# ── Stage 1: Build C++ timeline service ────────────────────────────────────────
FROM debian:trixie AS cpp-builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake ninja-build git pkg-config ca-certificates \
    libdrogon-dev \
    libjsoncpp-dev uuid-dev zlib1g-dev \
    libpqxx-dev libpq-dev \
    libspdlog-dev \
    libyaml-cpp-dev \
    nlohmann-json3-dev \
    libcurl4-openssl-dev \
    libsqlite3-dev libbrotli-dev libhiredis-dev \
    default-libmysqlclient-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY yolo_detection_cpp/ yolo_detection_cpp/

RUN cmake -S yolo_detection_cpp -B yolo_detection_cpp/build \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=OFF \
    && cmake --build yolo_detection_cpp/build --target yolo_timeline

# ── Stage 2: Build Angular frontend ───────────────────────────────────────────
FROM node:20-slim AS frontend-builder

WORKDIR /app
COPY frontend/package.json frontend/package-lock.json ./
RUN npm ci
COPY frontend/ ./
RUN npx ng build --configuration production --base-href /

# ── Stage 3: Runtime ──────────────────────────────────────────────────────────
FROM ${BUILD_FROM}

RUN apt-get update && apt-get install -y --no-install-recommends \
    libdrogon1t64 libtrantor1 \
    libpqxx-7.10 libpq5 \
    libyaml-cpp0.8 libjsoncpp26 \
    libspdlog1.15 libfmt10 \
    libuuid1 libbrotli1 libsqlite3-0 libhiredis1.1.0 libmariadb3 \
    libssl3 libkrb5-3 \
    libcurl4t64 \
    jq curl \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=cpp-builder \
    /build/yolo_detection_cpp/build/services/timeline/yolo_timeline \
    /usr/local/bin/yolo_timeline
RUN chmod +x /usr/local/bin/yolo_timeline

COPY --from=frontend-builder /app/dist/browser/ /app/static/
RUN cp /app/static/index.html /app/static/index.html.template

COPY ha-addon/run.sh /run.sh
RUN chmod a+x /run.sh

CMD ["/run.sh"]
