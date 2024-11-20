// small, fast 64 bit hash function, streaming api version.
//
// usage:
//     ChibiHash64Ctx ctx = chibihash64_init(seed);
//     chibihash64_append(&ctx, "hello, ", 7);
//     chibihash64_append(&ctx, "world!", 6);
//     uint64_t hash = chibihash64_finish(&ctx);
//
// https://github.com/N-R-K/ChibiHash
// https://nrk.neocities.org/articles/chibihash
//
// This is free and unencumbered software released into the public domain.
// For more information, please refer to <https://unlicense.org/>
#pragma once
#include <stdint.h>
#include <stddef.h>

typedef struct {
	uint64_t h[4];
	uint8_t  buf[32];
	int      buf_len;
	uint64_t total_len;
	uint64_t seed;
} ChibiHash64Ctx;

/////////

const uint64_t CHIBIHASH64__P1 = UINT64_C(0x2B7E151628AED2A5);
const uint64_t CHIBIHASH64__P2 = UINT64_C(0x9E3793492EEDC3F7);
const uint64_t CHIBIHASH64__P3 = UINT64_C(0x3243F6A8885A308D);

static inline uint64_t
chibihash64___load64le(const uint8_t *p)
{
	return (uint64_t)p[0] <<  0 | (uint64_t)p[1] <<  8 |
	       (uint64_t)p[2] << 16 | (uint64_t)p[3] << 24 |
	       (uint64_t)p[4] << 32 | (uint64_t)p[5] << 40 |
	       (uint64_t)p[6] << 48 | (uint64_t)p[7] << 56;
}

static ChibiHash64Ctx
chibihash64_init(uint64_t seed)
{
	ChibiHash64Ctx ctx = {0};
	ctx.h[0] = CHIBIHASH64__P1;
	ctx.h[1] = CHIBIHASH64__P2;
	ctx.h[2] = CHIBIHASH64__P3;
	ctx.h[3] = seed;
	ctx.seed = seed;
	return ctx;
}

static void
chibihash64_append(ChibiHash64Ctx *restrict ctx, void *restrict keyIn, ptrdiff_t len)
{
	const uint8_t *restrict p = keyIn;
	size_t l = len;
	// if there's data in ctx->buf, try to fill it up
	if (ctx->buf_len > 0) for (; l > 0 && ctx->buf_len < 32; ++p, --l) {
		ctx->buf[ctx->buf_len++] = *p;
	}
	// flush it if it's filled
	if (ctx->buf_len == 32) {
		for (int i = 0; i < 4; ++i) {
			uint64_t lane = chibihash64___load64le(ctx->buf + (i*8));
			ctx->h[i] ^= lane;
			ctx->h[i] *= CHIBIHASH64__P1;
			ctx->h[(i+1)&3] ^= ((lane << 40) | (lane >> 24));
		}
		ctx->buf_len = 0;
	}
	// process stripes, no copy
	for (; l >= 32; l -= 32) {
		for (int i = 0; i < 4; ++i, p += 8) {
			uint64_t lane = chibihash64___load64le(p);
			ctx->h[i] ^= lane;
			ctx->h[i] *= CHIBIHASH64__P1;
			ctx->h[(i+1)&3] ^= ((lane << 40) | (lane >> 24));
		}
	}
	// tail end of the input goes to the buffer
	for (; l > 0; ++p, --l) {
		ctx->buf[ctx->buf_len++] = *p;
	}
	ctx->total_len += len;
}

static uint64_t
chibihash64_finish(ChibiHash64Ctx *restrict ctx)
{
	ctx->h[0] += (ctx->total_len << 32) | (ctx->total_len >> 32);
	ptrdiff_t l = ctx->buf_len;
	uint8_t *restrict p = ctx->buf;
	if (l & 1) {
		ctx->h[0] ^= p[0];
		--l, ++p;
	}
	ctx->h[0] *= CHIBIHASH64__P2; ctx->h[0] ^= ctx->h[0] >> 31;

	for (int i = 1; l >= 8; l -= 8, p += 8, ++i) {
		ctx->h[i] ^= chibihash64___load64le(p);
		ctx->h[i] *= CHIBIHASH64__P2; ctx->h[i] ^= ctx->h[i] >> 31;
	}

	for (int i = 0; l > 0; l -= 2, p += 2, ++i) {
		ctx->h[i] ^= (p[0] | ((uint64_t)p[1] << 8));
		ctx->h[i] *= CHIBIHASH64__P3; ctx->h[i] ^= ctx->h[i] >> 31;
	}

	uint64_t x = ctx->seed;
	x ^= ctx->h[0] * ((ctx->h[2] >> 32)|1);
	x ^= ctx->h[1] * ((ctx->h[3] >> 32)|1);
	x ^= ctx->h[2] * ((ctx->h[0] >> 32)|1);
	x ^= ctx->h[3] * ((ctx->h[1] >> 32)|1);

	// moremur: https://mostlymangling.blogspot.com/2019/12/stronger-better-morer-moremur-better.html
	x ^= x >> 27; x *= UINT64_C(0x3C79AC492BA7B653);
	x ^= x >> 33; x *= UINT64_C(0x1C69B3F74AC4AE35);
	x ^= x >> 27;

	return x;
}
