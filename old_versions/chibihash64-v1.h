// small, fast 64 bit hash function (version 1).
//
// https://github.com/N-R-K/ChibiHash
// https://nrk.neocities.org/articles/chibihash
//
// This is free and unencumbered software released into the public domain.
// For more information, please refer to <https://unlicense.org/>
#pragma once
#include <stdint.h>
#include <stddef.h>

static inline uint64_t
chibihash64__load64le(const uint8_t *p)
{
	return (uint64_t)p[0] <<  0 | (uint64_t)p[1] <<  8 |
	       (uint64_t)p[2] << 16 | (uint64_t)p[3] << 24 |
	       (uint64_t)p[4] << 32 | (uint64_t)p[5] << 40 |
	       (uint64_t)p[6] << 48 | (uint64_t)p[7] << 56;
}

static inline uint64_t
chibihash64(const void *keyIn, ptrdiff_t len, uint64_t seed)
{
	const uint8_t *k = (const uint8_t *)keyIn;
	ptrdiff_t l = len;

	const uint64_t P1 = UINT64_C(0x2B7E151628AED2A5);
	const uint64_t P2 = UINT64_C(0x9E3793492EEDC3F7);
	const uint64_t P3 = UINT64_C(0x3243F6A8885A308D);

	uint64_t h[4] = { P1, P2, P3, seed };

	// unrolling gives very slight speed boost on large inputs at the cost
	// of larger code size. typically not worth the trade off as larger
	// code-size hinders inlinability as well
	// #pragma GCC unroll 2
	for (; l >= 32; l -= 32) {
		for (int i = 0; i < 4; ++i, k += 8) {
			uint64_t lane = chibihash64__load64le(k);
			h[i] ^= lane;
			h[i] *= P1;
			h[(i+1)&3] ^= ((lane << 40) | (lane >> 24));
		}
	}

	h[0] += ((uint64_t)len << 32) | ((uint64_t)len >> 32);
	if (l & 1) {
		h[0] ^= k[0];
		--l, ++k;
	}
	h[0] *= P2; h[0] ^= h[0] >> 31;

	for (int i = 1; l >= 8; l -= 8, k += 8, ++i) {
		h[i] ^= chibihash64__load64le(k);
		h[i] *= P2; h[i] ^= h[i] >> 31;
	}

	for (int i = 0; l > 0; l -= 2, k += 2, ++i) {
		h[i] ^= (k[0] | ((uint64_t)k[1] << 8));
		h[i] *= P3; h[i] ^= h[i] >> 31;
	}

	uint64_t x = seed;
	x ^= h[0] * ((h[2] >> 32)|1);
	x ^= h[1] * ((h[3] >> 32)|1);
	x ^= h[2] * ((h[0] >> 32)|1);
	x ^= h[3] * ((h[1] >> 32)|1);

	// moremur: https://mostlymangling.blogspot.com/2019/12/stronger-better-morer-moremur-better.html
	x ^= x >> 27; x *= UINT64_C(0x3C79AC492BA7B653);
	x ^= x >> 33; x *= UINT64_C(0x1C69B3F74AC4AE35);
	x ^= x >> 27;

	return x;
}
