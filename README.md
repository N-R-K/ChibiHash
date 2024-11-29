# Work in progress v2 branch

This is a WIP branch for version 2 of ChibiHash.
Things may change at any point without notice.

Some major improvement compared to v1:

- Faster performance on short string (49 cycles/hash vs 34 cycles/hash).
  The tail end handling has been reworked entirely with some inspiration from
  wyhash's short input reading.
- Better seeding. v1 seed only affected 64 bits of the initial state.
  v2 seed affects the full 256 bits. This allows it to pass smhasher3's
  SeedBlockLen and SeedBlockOffset tests.
- Slightly better mixing in bulk handling.
- Passes all 252 tests in smhasher3 (commit 34093a3), v1 failed 3.

Avenue for improvement:

- Faster bulk handling without using 128 bit multiplication.

---

Below is the original v1 readme, unaltered.

# ChibiHash: Small, Fast 64 bit hash function

I started writing this because all the 64 bit hash functions I came across were
either too slow (FNV-1a, one byte at a time processing), or too large spanning
hundreds of lines of code, or non-portable due to using hardware specific
instructions.
Being small and portable, the goal is to be able to use ChibiHash as a good
"default" for non-cryptographic 64-bit hashing needs.

Some key features:

* Small: ~60 loc in C
* Fast: See benchmark table below
* Portable: Doesn't use hardware specific instructions (e.g SSE)
* Good Quality: Passes [smhasher][], so should be good quality (I think)
* Unencumbered: Released into the public domain
* Free of undefined behavior and gives same result regardless of host system's endianness.
* Non-cryptographic

Here's some benchmark against other similar hash functions:

| Name |      Large input (GiB/sec)  |  Small input (Cycles/Hash) |
| :--- | :-------------------------: | :------------------------: |
| chibihash64  |  **18.08**   |   49   |
| xxhash64     |    12.59     |   50   |
| city64       |    14.95     | **35** |
| spooky64     |    13.83     |   59   |

It's the fastest of the bunch for large string throughput.
For small string (< 32 bytes), cityhash beats it - worth noting that cityhash
has [hardcoded special cases][city-small] for input below or equal 32 bytes.

[smhasher]: https://github.com/aappleby/smhasher
[city-small]: https://github.com/google/cityhash/blob/f5dc54147fcce12cefd16548c8e760d68ac04226/src/city.cc#L367-L375

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
