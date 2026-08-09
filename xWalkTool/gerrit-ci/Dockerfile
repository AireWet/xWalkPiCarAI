FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install --yes --no-install-recommends \
        ca-certificates \
        clang \
        clang-tidy \
        cmake \
        cppcheck \
        g++ \
        gcc \
        gcovr \
        git \
        libasound2-dev \
        libcurl4-openssl-dev \
        libgrpc++-dev \
        libgtest-dev \
        libprotobuf-dev \
        libtinyxml2-dev \
        libyaml-cpp-dev \
        ninja-build \
        openssh-client \
        protobuf-compiler \
        protobuf-compiler-grpc \
        python3 \
        python3-nacl \
        python3-requests \
        shellcheck \
    && rm -rf /var/lib/apt/lists/*

COPY xWalkGerritCi.py /usr/local/bin/xWalkGerritCi.py
COPY xWalkGerritLogServer.py /usr/local/bin/xWalkGerritLogServer.py
COPY xWalkGerritQuality.py /usr/local/bin/xWalkGerritQuality.py

RUN chmod 0555 /usr/local/bin/xWalkGerritCi.py \
        /usr/local/bin/xWalkGerritLogServer.py \
        /usr/local/bin/xWalkGerritQuality.py \
    && mkdir --parents /var/lib/xwalk-gerrit-ci /var/log/xwalk-gerrit-ci \
    && chown --recursive ubuntu:ubuntu \
        /var/lib/xwalk-gerrit-ci /var/log/xwalk-gerrit-ci

USER ubuntu

ENTRYPOINT ["python3", "/usr/local/bin/xWalkGerritCi.py"]
