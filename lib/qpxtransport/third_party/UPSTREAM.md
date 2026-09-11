# Vendored cryptographic primitives

These sources implement only the legacy Plextor challenge-response protocol. MD5 and Blowfish are not suitable for
new security protocols.

## OpenBSD Blowfish

- Upstream: OpenBSD `src`, commit `f6ee91c65ae8090d63bb0756f3a80a2af2e97158`
- Imported: 2026-08-17
- Source files:
  - `lib/libc/crypt/blowfish.c`
  - `include/blf.h`
- URLs:
  - <https://raw.githubusercontent.com/openbsd/src/f6ee91c65ae8090d63bb0756f3a80a2af2e97158/lib/libc/crypt/blowfish.c>
  - <https://raw.githubusercontent.com/openbsd/src/f6ee91c65ae8090d63bb0756f3a80a2af2e97158/include/blf.h>
- License: 3-clause BSD license by Niels Provos, included in both files.
- Upstream SHA-256:
  - `blowfish.c`: `f2dfc3245ff75a9deb744e826bc7b99c6fca156ff8f395382c378722f27370af`
  - `blf.h`: `6e8d437acedf7f27b6547ce036e02c7825c3235fb9f7e55c79cf496063deb1db`

`blowfish.c` is unmodified. QPxTool adds a clearly marked shim to `blf.h` for non-BSD integer types, makes OpenBSD's
libc-only `DEF_WEAK` annotation a no-op, and prefixes external symbols with `qpx_openbsd_` to prevent collisions.
The vendored `blf.h` SHA-256 after those changes is
`3f2fbe8561ba2be94ed5d4b608d2458b600a974a25ca92b0da2492e0ae051400`.

## Openwall MD5

- Upstream: Solar Designer's public-domain source-code collection
- Imported: 2026-08-17
- URLs:
  - <https://openwall.info/wiki/_media/people/solar/software/public-domain-source-code/md5.c>
  - <https://openwall.info/wiki/_media/people/solar/software/public-domain-source-code/md5.h>
- License: public-domain dedication with a permissive redistribution fallback, included in both files.
- Upstream SHA-256:
  - `md5.c`: `0eb851f59869e6e4e23ccbaff30ba1874c1a0e5a00d3cfef7529561dfcee2550`
  - `md5.h`: `2f7fd79f1a8ec20e8f7febccbc445eddc25a654f4d6503f289561178e5aea781`

`md5.c` is unmodified. QPxTool adds a clearly marked shim to `md5.h` that prefixes external symbols with
`qpx_openwall_` to prevent collisions. The vendored `md5.h` SHA-256 after that change is
`ea14a437be106375a217c9a312beb1686ea3cd4d74a9f74406d1c7d4a93b0af1`.

When updating either implementation, retrieve the named files from upstream, verify their hashes before applying the
documented header shims, and rerun the MD5, Blowfish, and Plextor compatibility vectors.

## Verification vectors

- MD5 of ASCII `abc`: `900150983cd24fb0d6963f7d28e17f72` (RFC 1321)
- Blowfish ECB with an all-zero 8-byte key and all-zero plaintext: `4ef997456198dd78` (Schneier vector)
- Plextor response for challenge bytes `000102030405060708090a0b0c0d0e0f`:
  `0b9921eb0fe3bcb556088c1f76fa22d1` (captured from the implementation replaced by this import)
