#include <assert.h>
#include <stdint.h>
#include <stddef.h>

#include "../chibihash64.h"
#include "../chibihash64-stream.h"

static uint32_t
rng32(uint64_t *state)
{
	uint64_t old = *state;

	*state *= UINT64_C(1111111111111111111);
	*state += UINT64_C(0x1337);

	int r = old >> (64 - 5);
	uint64_t x = (old ^ (old >> 18)) >> (32 - 5);
	x = (x >> (-r & 31)) | (x << r);
	return (uint32_t)x;
}

static uint64_t
stream_hash_chunks(uint8_t *data, ptrdiff_t len, uint64_t seed, ptrdiff_t chunk_len)
{
	ChibiHash64Ctx ctx = chibihash64_init(seed);
	ptrdiff_t off = 0;

	while (off < len) {
		ptrdiff_t n = len - off;
		if (n > chunk_len) {
			n = chunk_len;
		}
		chibihash64_append(&ctx, data + off, n);
		off += n;
	}

	return chibihash64_finish(&ctx);
}

static void
check_all_splits(uint8_t *data, ptrdiff_t len, uint64_t seed)
{
	uint64_t expected = chibihash64(data, len, seed);

	for (ptrdiff_t cut = 0; cut <= len; ++cut) {
		ChibiHash64Ctx ctx = chibihash64_init(seed);
		chibihash64_append(&ctx, data, cut);
		chibihash64_append(&ctx, data + cut, len - cut);
		assert(chibihash64_finish(&ctx) == expected);
	}

	ChibiHash64Ctx ctx = chibihash64_init(seed);
	for (ptrdiff_t i = 0; i < len; ++i) {
		chibihash64_append(&ctx, data + i, 1);
	}
	assert(chibihash64_finish(&ctx) == expected);
}

int
main(void)
{
	static const uint64_t seeds[] = {
		UINT64_C(0),
		UINT64_C(1),
		UINT64_C(55555),
		UINT64_C(0x0123456789ABCDEF),
	};
	uint8_t data[4096];
	uint64_t rngstate = UINT64_C(55555);

	for (size_t i = 0; i < sizeof(data); ++i) {
		data[i] = (uint8_t)rng32(&rngstate);
	}

	for (size_t i = 0; i < sizeof(seeds) / sizeof(seeds[0]); ++i) {
		ChibiHash64Ctx ctx = chibihash64_init(seeds[i]);
		chibihash64_append(&ctx, data, 31);
		chibihash64_append(&ctx, data, 0);
		chibihash64_append(&ctx, data + 31, 33);
		assert(chibihash64(data, 64, seeds[i]) == chibihash64_finish(&ctx));
	}

	for (size_t i = 0; i < sizeof(seeds) / sizeof(seeds[0]); ++i) {
		for (ptrdiff_t len = 0; len <= 256; ++len) {
			check_all_splits(data, len, seeds[i]);
		}

		for (ptrdiff_t chunk_len = 1; chunk_len <= 65; ++chunk_len) {
			uint64_t expected = chibihash64(data, (ptrdiff_t)sizeof(data), seeds[i]);
			assert(stream_hash_chunks(data, (ptrdiff_t)sizeof(data), seeds[i], chunk_len) == expected);
		}
	}

	return 0;
}
