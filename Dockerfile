FROM ubuntu:22.04

# Install minimal build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /build

# Copy source code
COPY . .

# Build the project
RUN echo "Building art2img..." && make

# Test the build
RUN echo "Running tests..." && make test

# Default command runs the tool
CMD ["./bin/art2img", "--help"]