ARG BUILD_FROM=debian:trixie-slim
FROM ${BUILD_FROM}

RUN apt-get update && apt-get install -y --no-install-recommends \
    libdrogon1t64 libtrantor1 \
    libpqxx-7.10 libpq5 \
    libyaml-cpp0.8 libjsoncpp26 \
    libspdlog1.15 libfmt10 \
    libuuid1 libbrotli1 libsqlite3-0 libhiredis1.1.0 libmariadb3 \
    libssl3 libkrb5-3 \
    libcurl4t64 \
    libpaho-mqtt1.3 libpaho-mqttpp3-1 \
    jq curl \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Pre-built binary (compiled on NVR and copied to add-on dir)
COPY yolo_timeline /usr/local/bin/yolo_timeline
RUN chmod +x /usr/local/bin/yolo_timeline

# Pre-built Angular frontend (compiled on NVR and copied to add-on dir)
COPY static/ /app/static/
RUN cp /app/static/index.html /app/static/index.html.template

COPY run.sh /run.sh
RUN chmod a+x /run.sh

CMD ["/run.sh"]
