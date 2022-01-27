libacquire
=======
[![License](https://img.shields.io/badge/license-Apache--2.0%20OR%20MIT-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![CI for Linux, Windows, macOS](https://github.com/offscale/libacquire/workflows/CI%20for%20Linux,%20Windows,%20macOS/badge.svg)](https://github.com/offscale/libacquire/actions)
[![CI for FreeBSD](https://api.cirrus-ci.com/github/offscale/libacquire.svg)](https://cirrus-ci.com/github/offscale/libacquire)
[![C89](https://img.shields.io/badge/C-89-blue)](https://en.wikipedia.org/wiki/C89_(C_version))

The core for your package manager, minus the dependency graph components. Features: **download**, **verify**, and **extract**.

By default—for HTTP, HTTPS, and FTP—this uses `libfetch` on FreeBSD; `wininet` on Windows; and `libcurl` everywhere else. Override with `-DUSE_LIBCURL` or  `-DUSE_LIBFETCH`.

By default—for MD5, SHA256, SHA512—this uses `wincrypt` on Windows; and `OpenSSL` everywhere else. _Note that on macOS this uses the builtin `CommonCrypto/CommonDigest.h` header, and on OpenBSD it uses `LibreSSL`; however in both of these cases it's the OpenSSL API with different headers._ Override with `-DUSE_OPENSSL`.

By default—for crc32c—this uses `rhash` if available (also giving access to: CRC32, MD4, MD5, SHA1, SHA256, SHA512, SHA3, AICH, ED2K, DC++ TTH, BitTorrent BTIH, Tiger, GOST R 34.11-94, GOST R 34.11-2012, RIPEMD-160, HAS-160, EDON-R, and Whirlpool); otherwise uses included crc32c implementation. Override with `-DUSE_CRC32C`.

By default—for decompression—this uses `compressapi.h` on Windows; then, in order of precedence tries: libarchive, zlib, or downloads miniz.

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

## Library dependencies

Dynamically links to shared libraries, defaulting to what's already installed on your OS by default.

If your OS doesn't have the dependency, an optimised dependency free version will be `add_library`'d and depended upon.

*† default on that OS*

*\* WiP, not usable yet*

### Cryptography

  | API     | Package enable flag | OS support |
  | ------- | ------------------- | ---------- |
  | [`wincrypt.h`](https://docs.microsoft.com/en-us/windows/win32/api/wincrypt) | `USE_WINCRYPT`  | Windows †
  | [OpenSSL](https://openssl.org) | `USE_COMMON_CRYPTO` | macOS † |
  | [OpenSSL](https://openssl.org) | `USE_LIBRESSL`      | All that [LibreSSL](https://libressl.org) supports  |
  | [OpenSSL](https://openssl.org) | `USE_OPENSSL`       | All that [OpenSSL](https://openssl.org) supports; default † on non macOS and Windows |

(will fallback to checksum library if undefined and only hashing is required and checksum library defined isn't CRC32C)

### Networking

  | API     | Package enable flag | OS support |
  | ------- | ------------------- | ---------- |
  | [WinINet](https://docs.microsoft.com/en-us/windows/win32/wininet) | `USE_WININET` | Windows † | 
  | [libfetch](https://www.freebsd.org/cgi/man.cgi?fetch(3)) | `USE_LIBFETCH`  | FreeBSD † & derivatives; other OS support WiP (try `BUILD_FREEBSD_LIBFETCH`)
  | \* [OpenBSD's `ftp`](https://man.openbsd.org/ftp.1) | `USE_OPENBSD_FTP`  | OpenBSD † (try `BUILD_OPENBSD_FTP`)
  | [libcurl](https://curl.se/libcurl)  | `USE_LIBCURL`  | All that [`curl`](https://curl.se) supports; default † on non macOS and Windows

### Extraction

  | API     | Package enable flag | OS support |
  | ------- | ------------------- | ---------- |
  | \* [`compressapi.h`](https://docs.microsoft.com/en-us/windows/win32/api/_cmpapi) | `USE_WINCOMPRESSAPI` | Windows †
  | \* [zlib](https://zlib.net) | `USE_ZLIB` | All that zlib supports; default † (if installed) on macOS, Linux, BSD, and SunOS
  | \* [libarchive](https://libarchive.org) | `USE_LIBARCHIVE` | All that libarchive supports
  | [miniz](https://github.com/richgel999/miniz) with [zip](https://github.com/kuba--/zip) API | `USE_MINIZ` | All that miniz + zip supports; default † fallback

### Checksum
Note that most checksum libraries are crypto libraries so working with these APIs isn't required for libacquire:

  | API                                                           | Package enable flag | OS support                      |
  ----------------------------------------------------------------| ------------------- |---------------------------------|
  | \* `acquire_crc32c.h`                                         | `USE_CRC32C`        | All                             |
  | \* [RHash (Recursive Hasher)](https://github.com/rhash/RHash) | `USE_LIBRHASH`      | All; † fallback to `USE_CRC32C` 

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

### Dependencies

  - [CMake](https://cmake.org) (3.19 or later)
  - C compiler (any that work with CMake, and were released within the last 30 years)
  - Crypto, HTTPS, and unarchiving library (see above for what to override and OS support; defaults to OS builtin API)

### Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### CLI interface

Generated with `docopt`, use `python -m pip install docopt-c` then:
```bash
$ python -m docopt_c '.docopt' -o 'libacquire/acquire/cli'
```

## See also

  - https://github.com/SamuelMarks/curl-simple-https

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
