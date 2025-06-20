FROM alpine

RUN apk add --no-cache \
    bash \
    build-base \
    cmake \
    curl \
    curl-dev \
    g++ \
    gcc \
    git \
    libarchive-dev \
    libbsd-dev \
    linux-headers \
    make \
    musl-dev \
    ninja-build \
    openssl-dev \
    perl \
    pkgconf \
    python3 \
    rhash-dev \
    sudo \
    unzip \
    zip && \
    ln -s /usr/lib/ninja-build/bin/ninja /usr/bin/ninja
WORKDIR /
RUN git clone --depth=1 --single-branch https://github.com/offscale/vcpkg -b project0 && \
    cd vcpkg && \
    ./bootstrap-vcpkg.sh -disableMetrics && \
    ./vcpkg install kubazip libarchive rhash

COPY . /libacquire

WORKDIR /libacquire/build

RUN cmake -DCMAKE_BUILD_TYPE='Debug' \
          -DCMAKE_TOOLCHAIN_FILE='/vcpkg/scripts/buildsystems/vcpkg.cmake' \
          .. && \
    cmake --build .

CMD ctest .

# ENTRYPOINT ["/libacquire/build/acquire_cli", "--version"]
