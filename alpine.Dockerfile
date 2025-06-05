FROM alpine

RUN apk add --no-cache \
    cmake \
    curl-dev \
    gcc \
    libarchive-dev \
    libbsd-dev \
    rhash-dev \
    linux-headers \
    make \
    musl-dev \
    openssl-dev \
    pkgconf

COPY . /libacquire

WORKDIR /libacquire/build

RUN cmake -DCMAKE_BUILD_TYPE="Debug" .. && \
    cmake --build .

CMD ctest .

ENTRYPOINT ["/libacquire/build/acquire", "--version"]
