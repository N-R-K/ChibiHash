#ifndef CHIBIHASH64__STREAM_HGUARD
#define CHIBIHASH64__STREAM_HGUARD
// small, fast 64 bit hash function (version 2), with streaming api.
//
// https://github.com/N-R-K/ChibiHash
//
// Usage
// =====
// ChibiHash64Ctx ctx = chibihash64_init(seed);
// chibihash64_append(&ctx, "hello, ", 7);
// chibihash64_append(&ctx, "world!", 6);
// uint64_t hash = chibihash64_finish(&ctx);
//
// This is free and unencumbered software released into the public domain.
// For more information, please refer to <https://unlicense.org/>

typedef struct {
	uint64_t  h[4];
	uint8_t   buf[32];
	int       buf_len;
	ptrdiff_t total_len;
	uint64_t  seed;
} ChibiHash64Ctx;

// hacky way to make both chibihash64.h and chibihash64-stream.h usable within same TU
#if !defined(CHIBIHASH64__HGUARD)
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
#endif

static inline void
chibihash64__stream_block(uint64_t h[4], const uint8_t *p)
{
	const uint64_t K = UINT64_C(0x2B7E151628AED2A7);

	for (int i = 0; i < 4; ++i, p += 8) {
		uint64_t stripe = chibihash64__load64le(p);
		h[i] = (stripe + h[i]) * K;
		h[(i+1)&3] += chibihash64__rotl(stripe, 27);
	}
}

static inline ChibiHash64Ctx
chibihash64_init(uint64_t seed)
{
	const uint64_t K = UINT64_C(0x2B7E151628AED2A7); // digits of e
	uint64_t seed2 = chibihash64__rotl(seed-K, 15) + chibihash64__rotl(seed-K, 47);
	ChibiHash64Ctx ctx = {
		.h = { seed, seed+K, seed2, seed2+(K*K^K) },
		.seed = seed,
	};
	return ctx;
}

static inline void
chibihash64_append(ChibiHash64Ctx *restrict ctx, void *restrict keyIn, ptrdiff_t len)
{
	const uint8_t *restrict p = keyIn;
	ptrdiff_t l = len;

	ctx->total_len += len;

	for (; l > 0 && ctx->buf_len < 32; --l, ++p) {
		ctx->buf[ctx->buf_len++] = *p;
	}

	if (ctx->buf_len == 32) {
		chibihash64__stream_block(ctx->h, ctx->buf);
		ctx->buf_len = 0;
	}

	for (; l >= 32; l -= 32, p += 32) {
		chibihash64__stream_block(ctx->h, p);
	}

	for (; l > 0; --l, ++p) {
		ctx->buf[ctx->buf_len++] = *p;
	}
}

static inline uint64_t
chibihash64_finish(ChibiHash64Ctx *restrict ctx)
{
	const uint64_t K = UINT64_C(0x2B7E151628AED2A7);
	const uint8_t *p = ctx->buf;
	ptrdiff_t l = ctx->buf_len;
	uint64_t h[4] = { ctx->h[0], ctx->h[1], ctx->h[2], ctx->h[3] };

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

	h[0] += chibihash64__rotl(h[2] * K, 31) ^ (h[2] >> 31);
	h[1] += chibihash64__rotl(h[3] * K, 31) ^ (h[3] >> 31);
	h[0] *= K; h[0] ^= h[0] >> 31;
	h[1] += h[0];

	uint64_t x = (uint64_t)ctx->total_len * K;
	x ^= chibihash64__rotl(x, 29);
	x += ctx->seed;
	x ^= h[1];

	x ^= chibihash64__rotl(x, 15) ^ chibihash64__rotl(x, 42);
	x *= K;
	x ^= chibihash64__rotl(x, 13) ^ chibihash64__rotl(x, 31);

	return x;
}

#endif // CHIBIHASH64__STREAM_HGUARD
