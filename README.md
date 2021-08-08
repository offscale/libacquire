libacquire
=======
[![License](https://img.shields.io/badge/license-Apache--2.0%20OR%20MIT-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![CI for Linux, Windows, macOS](https://github.com/offscale/libacquire/workflows/CI%20for%20Linux,%20Windows,%20macOS/badge.svg)](https://github.com/offscale/libacquire/actions)
[![CI for FreeBSD](https://api.cirrus-ci.com/github/offscale/libacquire.svg)](https://cirrus-ci.com/github/offscale/libacquire)
[![C89](https://img.shields.io/badge/C-89-blue)](https://en.wikipedia.org/wiki/C89_(C_version))

The core for your package manager, minus the dependency graph components. Features: **download**, **verify**, and **extract**.

By default—for HTTP, HTTPS, and FTP—this uses `libfetch` on FreeBSD; `wininet` on Windows; and `libcurl` everywhere else. Override with `-DUSE_LIBCURL` or  `-DUSE_LIBFETCH`.

By default—for MD5, SHA256, SHA512—this uses `wincrypt` on Windows; and `OpenSSL` everywhere else. _Note that on macOS this uses the builtin `CommonCrypto/CommonDigest.h` header, and on OpenBSD it uses `LibreSSL`; however in both of these cases it's the OpenSSL API with different headers._ Override with `-DUSE_OPENSSL`.

Supports:

  - [ ] Linux
  - [ ] macOS
  - [ ] Windows
  - [ ] Solaris/OpenSolaris/illumos
  - [ ] BSD

## Advantage

  - Extremely portable C code (ANSI C: C89)
  - link with different libraries, including those built in to your OS
  - very small executable size (thanks to above)
  - fast
  - simple API, easy to integrate in your C project (or really any language, they all have nice FFI back to C or a C intermediary language)
  - default cipher selection >= TLS 1.2

## Docker

`Dockerfile`s are provided for convenience, try them out, e.g., by running:

    docker build . -f Dockerfile.alpine --tag libacquire
    docker run libacquire

## Shell script equivalent (UNIX with `grep`, `curl`, and `tar`)

```sh
#!/usr/bin/env sh

function acquire () {
    target_directory="$1"
    url="$2"
    checksum="$3"
    prev_wd="`pwd`"
    filename=`echo "$url" | grep -oE "[^/]+$"`

    cd "$target_directory"
    curl -sS --tlsv1.2 -O "$url"
    printf '%s %s' "$checksum" "$filename" | sha256sum -c --quiet
    tar xf "$filename"
    cd "$prev_wd"
}

acquire \
  '/tmp' \
  'https://nodejs.org/download/release/v12.16.3/node-v12.16.3-headers.tar.xz' \
  '27169e5284a7bc2783bfb79bb80f69453cc59d2d5ca3d5d22265826370475cad'
```

### Advantages

  - Works in most shells.
  - Easy to read
  - Can be edited easily

### Disadvantages

  - Doesn't work on Windows
  - Everyone has their own equivalent script as above; followed by `install`, `cpio`, or just manually `mkdir -p`, `cp`, `chown`, and `chmod`
  - Needs to be edited for each new version of Node.js (still a disadvantage of libacquire; but wait till you see the `nvm` style independent package managers built with libacquire!)
  - Everyone has their favourite `curl`/`wget`/`fetch` command, rarely do they set the ciphers (TLS 1.2 or higher is now the recommendation)
  - Can be edited easily

## Developer note

Want different options for libcurl, OpenSSL, or any of the other dependencies? - CMake has a `CACHE`ing mechanism. - You should be able to explicitly include your settings before including `libacquire`, and it'll use the one already included (with your custom settings).

---

## License

Licensed under either of

- Apache License, Version 2.0 ([LICENSE-APACHE](LICENSE-APACHE) or <https://www.apache.org/licenses/LICENSE-2.0>)
- MIT license ([LICENSE-MIT](LICENSE-MIT) or <https://opensource.org/licenses/MIT>)

at your option.

### Contribution

Unless you explicitly state otherwise, any contribution intentionally submitted
for inclusion in the work by you, as defined in the Apache-2.0 license, shall be
dual licensed as above, without any additional terms or conditions.
