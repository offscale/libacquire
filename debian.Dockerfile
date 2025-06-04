FROM debian

RUN apt-get update -qq && \
    apt-get install -y \
      cmake \
      gcc \
      libarchive-dev \
      libbsd-dev \
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
