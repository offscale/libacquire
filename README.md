libacquire
=======
[![License](https://img.shields.io/badge/license-Apache--2.0%20OR%20MIT-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![CI for Linux, Windows, macOS](https://github.com/offscale/libacquire/workflows/CI%20for%20Linux,%20Windows,%20macOS/badge.svg)](https://github.com/offscale/libacquire/actions)
[![CI for FreeBSD](https://api.cirrus-ci.com/github/offscale/libacquire.svg)](https://cirrus-ci.com/github/offscale/libacquire)

Downloads using libcurl—if not Windows or built with `USE_LIBCURL`—or Windows APIs.

Supports:

  - [ ] Linux
  - [ ] macOS
  - [ ] Windows
  - [ ] Solaris/OpenSolaris/illumos
  - [ ] BSD

## Docker

`Dockerfile`s are provided for convenience, try them out, e.g., by running:

    docker build . -f Dockerfile.ubuntu --tag libacquire
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

### libaquire advantage

  - C89 code that is extremely portable
  - link with different libraries, including those built in to your OS
  - very small executable size (thanks to above)
  - fast
  - simple API, easy to integrate in your C project (or really any language, they all have nice FFI back to C or a C intermediary language)
  - default cipher selection is >= TLS 1.2

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
