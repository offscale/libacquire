FROM debian

ADD 'https://github.com/Kitware/CMake/releases/download/v3.21.4/cmake-3.21.4-linux-x86_64.sh' '/tmp/cmake.sh'

RUN sh /tmp/cmake.sh --prefix='/usr/local' --skip-license

RUN apt-get update -qq && \
    apt-get install -y \
      gcc \
      libc-dev \
      libcurl4-openssl-dev \
      librhash-dev \
      libssl-dev \
      make \
      pkg-config

COPY . /libacquire

WORKDIR /libacquire/build

# RUN apt-get install -y ripgrep && cd / && rg -F 'char *strnstr' 2>/dev/null

RUN cmake -DCMAKE_BUILD_TYPE="Debug" .. && \
    cmake --build .

CMD ctest .

ENTRYPOINT ["/libacquire/build/acquire", "--help"]
