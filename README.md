# ChibiHash: Small, Fast 64 bit hash function

I started writing this because all the 64 bit hash functions I came across were
either too slow (FNV-1a, one byte at a time processing), or too large spanning
hundreds of lines of code, or non-portable due to using hardware specific
instructions.
Being small and portable, the goal is to be able to use ChibiHash as a good
"default" for non-cryptographic 64-bit hashing needs.

Some key features:

* Small: ~65 loc in C
* Fast: See benchmark table below
* Portable: Doesn't use hardware specific instructions (e.g SSE)
* Good Quality: Passes [smhasher][] and [smhasher3][], so should be good quality (I think)
* Unencumbered: Released into the public domain
* Free of undefined behavior and gives same result regardless of host system's endianness.
* Non-cryptographic

Here's some benchmark (made via smhasher3) against other similar themed hash functions:

| Name               |    Large input (GiB/sec)    |  Small input (Cycles/Hash) |
| :---               | :-------------------------: | :------------------------: |
| chibihash64                       |  **24.20**   |   34   |
| xxhash64                          |    15.10     |   50   |
| city64                            |    18.30     |   47   |
| spooky64                          |    16.68     |   70   |
| rapidhash.protected <sup>1</sup>  |    21.50     | **32** |
| polymur-hash <sup>1, 2</sup>      |    13.82     |   43   |

1. Requires compiler/cpu support for retrieving the full 128 bit result of a
   64x64 bit multiply.
2. Universal, but has a complicated seeding step.

[smhasher]: https://github.com/aappleby/smhasher
[smhasher3]: https://gitlab.com/fwojcik/smhasher3

## When NOT to use

The introduction should make it clear on why you'd want to use this.
Here are some reasons to avoid using this:

* For cryptographic purposes.
* For protecting against [collision attacks](https://en.wikipedia.org/wiki/Collision_attack) (SipHash is the recommended one for this purpose).
* When you need very strong probability against collisions: ChibiHash does very
  minimal amount of mixing compared to other hashes (e.g xxhash64). And so
  chances of collision should in theory be higher.

## Unofficial ports

A list of unofficial ports to other languages is [maintained here](https://github.com/N-R-K/ChibiHash/issues/4).

## Changelog

### v2

- Faster performance on short string (42 cycles/hash vs 34 cycles/hash).
  The tail end handling has been reworked entirely with some inspiration from
  wyhash's short input reading.
- Better seeding. v1 seed only affected 64 bits of the initial state.
  v2 seed affects the full 256 bits. This allows it to pass smhasher3's
  SeedBlockLen and SeedBlockOffset tests.
- Slightly better mixing in bulk handling.
- Passes all 252 tests in smhasher3 (commit 34093a3), v1 failed 3.
