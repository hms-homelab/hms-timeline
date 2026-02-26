# Multi-stage build for yolo-timeline service
# Expected image size: ~100-150MB

# ==============================================================================
# Stage 1: Build
# ==============================================================================
FROM debian:bookworm-slim AS build

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    ninja-build \
    git \
    curl \
    zip \
    unzip \
    tar \
    pkg-config \
    libpq-dev \
    libssl-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Install vcpkg
ENV VCPKG_ROOT=/opt/vcpkg
RUN git clone --depth 1 https://github.com/microsoft/vcpkg.git $VCPKG_ROOT \
    && $VCPKG_ROOT/bootstrap-vcpkg.sh -disableMetrics

WORKDIR /src

# Copy vcpkg manifest first for dependency caching
COPY yolo_detection_cpp/vcpkg.json .
RUN $VCPKG_ROOT/vcpkg install --triplet x64-linux

# Copy source code
COPY yolo_detection_cpp/ .

# Build timeline service
RUN cmake -B build \
    -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -G Ninja \
    && cmake --build build --target yolo_timeline -j$(nproc)

# ==============================================================================
# Stage 2: Runtime
# ==============================================================================
FROM debian:bookworm-slim AS runtime

RUN apt-get update && apt-get install -y --no-install-recommends \
    libpq5 \
    libssl3 \
    ca-certificates \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Create app user
RUN useradd -r -s /bin/false -m appuser

WORKDIR /app

# Copy binary
COPY --from=build /src/build/services/timeline/yolo_timeline /usr/local/bin/yolo_timeline

# Copy Angular frontend dist
COPY frontend/dist/browser/ /app/frontend/dist/browser/

# Default config directory (mount your config.yaml here)
RUN mkdir -p /app/config /app/logs && chown -R appuser:appuser /app

USER appuser

EXPOSE 8080

HEALTHCHECK --interval=30s --timeout=5s --start-period=10s --retries=3 \
    CMD curl -f http://localhost:8080/health || exit 1

ENTRYPOINT ["/usr/local/bin/yolo_timeline"]
CMD ["--config", "/app/config/config.yaml"]
