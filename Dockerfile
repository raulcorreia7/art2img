FROM ubuntu:22.04

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    g++-mingw-w64-x86-64 \
    make \
    && rm -rf /var/lib/apt/lists/*

# Set up mingw64 cross-compiler
RUN update-alternatives --set x86_64-w64-mingw32-gcc /usr/bin/x86_64-w64-mingw32-gcc-posix \
    && update-alternatives --set x86_64-w64-mingw32-g++ /usr/bin/x86_64-w64-mingw32-g++-posix

# Create build directory
WORKDIR /build

# Copy source code
COPY . .

# Build for all platforms
RUN make all-platforms

# Default command to show built files
CMD ["ls", "-la", "bin/"]