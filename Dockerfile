FROM ubuntu:22.04

LABEL maintainer="gAgent Project"
LABEL description="FIPA-compliant Multi-Agent Platform"

# Avoid prompts from apt
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    qtbase5-dev \
    libboost-all-dev \
    libconfig++-dev \
    clang-format \
    cppcheck \
    gdb \
    valgrind \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /workspace

# Copy project files
COPY . /workspace

# Build the project
RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc)

# Set environment
ENV LD_LIBRARY_PATH=/workspace/src_agent/build:$LD_LIBRARY_PATH

CMD ["/bin/bash"]
