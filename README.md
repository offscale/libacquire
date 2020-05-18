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
