FROM debian

RUN apt-get update -qq && \
    apt-get install -y \
      cmake \
      curl \
      g++ \
      gcc \
      git \
      libarchive-dev \
      libbsd-dev \
      libc-dev \
      libcurl4-openssl-dev \
      librhash-dev \
      libssl-dev \
      libstdc++ \
      linux-libc-dev \
      make \
      ninja-build \
      pkg-config \
      unzip

RUN git clone --depth=1 --single-branch https://github.com/offscale/vcpkg -b project0 && \
    cd vcpkg && \
    ./bootstrap-vcpkg.sh -disableMetrics && \
    ./vcpkg install kubazip libarchive rhash

COPY . /libacquire

WORKDIR /libacquire/build

# RUN apt-get install -y ripgrep && cd / && rg -F 'char *strnstr' 2>/dev/null

RUN cmake -DCMAKE_BUILD_TYPE='Debug' \
          -DCMAKE_TOOLCHAIN_FILE='/vcpkg/scripts/buildsystems/vcpkg.cmake' \
          .. && \
    cmake --build .

CMD ctest .

# ENTRYPOINT ["/libacquire/build/acquire_cli", "--help"]
