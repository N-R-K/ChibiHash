// small, fast 64 bit hash function (version 2).
//
// https://github.com/N-R-K/ChibiHash
// https://nrk.neocities.org/articles/chibihash
//
// This is free and unencumbered software released into the public domain.
// For more information, please refer to <https://unlicense.org/>
#pragma once
#include <stdint.h>
#include <stddef.h>

static inline uint64_t chibihash64__load32le(const uint8_t *p)
{
	return (uint64_t)p[0] <<  0 | (uint64_t)p[1] <<  8 |
	       (uint64_t)p[2] << 16 | (uint64_t)p[3] << 24;
}
static inline uint64_t chibihash64__load64le(const uint8_t *p)
{
	return chibihash64__load32le(p) | (chibihash64__load32le(p+4) << 32);
}
static inline uint64_t chibihash64__rotl(uint64_t x, int n)
{
	return (x << n) | (x >> (-n & 63));
}
static inline uint64_t chibihash64__reduce(uint64_t a, uint64_t b)
{
	uint64_t x = a * (chibihash64__rotl(b, 9) | 1);
	x ^= chibihash64__rotl(b, 41) + chibihash64__rotl(a, 31);
	return x;
}

static inline uint64_t
chibihash64(const void *keyIn, ptrdiff_t len, uint64_t seed)
{
	const uint8_t *p = (const uint8_t *)keyIn;
	ptrdiff_t l = len;

	const uint64_t K = UINT64_C(0x2B7E151628AED2A7); // digits of e
	uint64_t seed2 = chibihash64__rotl(seed-K, 15) + chibihash64__rotl(seed-K, 47);
	uint64_t h[4] = { seed, seed+K, seed2, seed2+(K*K^K) };

	// depending on your system unrolling might (or might not) make things
	// a tad bit faster on large strings. on my system, it actually makes
	// things slower.
	// generally speaking, the cost of bigger code size is usually not
	// worth the trade-off since larger code-size will hinder inlinability
	// but depending on your needs, you may want to uncomment the pragma
	// below to unroll the loop.
	//#pragma GCC unroll 2
	for (; l >= 32; l -= 32) {
		for (int i = 0; i < 4; ++i, p += 8) {
			uint64_t stripe = chibihash64__load64le(p);
			h[i] = (stripe + h[i]) * K;
			h[(i+1)&3] += chibihash64__rotl(stripe, 27);
		}
	}

	for (; l >= 8; l -= 8, p += 8) {
		h[0] ^= chibihash64__load32le(p+0); h[0] *= K;
		h[1] ^= chibihash64__load32le(p+4); h[1] *= K;
	}

	if (l >= 4) {
		h[2] ^= chibihash64__load32le(p);
		h[3] ^= chibihash64__load32le(p + l - 4);
	} else if (l > 0) {
		h[2] ^= p[0];
		h[3] ^= p[l/2] | ((uint64_t)p[l-1] << 8);
	}

	uint64_t x = (uint64_t)len * K;
	x ^= chibihash64__rotl(x, 29);
	x ^= chibihash64__reduce(h[0], h[1]);
	x ^= chibihash64__reduce(h[2], h[3]) + seed;

	// moremur: https://mostlymangling.blogspot.com/2019/12/stronger-better-morer-moremur-better.html
	x ^= x >> 27; x *= UINT64_C(0x3C79AC492BA7B653);
	x ^= x >> 33; x *= UINT64_C(0x1C69B3F74AC4AE35);
	x ^= x >> 27;

	return x;
}
