#pragma once
#include "config.h"

#ifdef HAVE_VISUAL_STUDIO
#include <intrin.h>
#endif

#include <cstdint>
#include <array>

class HData{
public:
	// source http://stackoverflow.com/a/25808559/2440195
	static size_t CountSetBits(uint8_t v){
		// source http://stackoverflow.com/a/25808559/2440195
		static const uint8_t NIBBLE_LOOKUP[16] = {
			0, 1, 1, 2, 1, 2, 2, 3,
			1, 2, 2, 3, 2, 3, 3, 4
		};

		return NIBBLE_LOOKUP[v & 0x0F] + NIBBLE_LOOKUP[v >> 4];
	}
	static size_t CountSetBits(uint16_t v){
		const uint8_t *tmp = reinterpret_cast<const uint8_t *>(&v);
		return CountSetBits(tmp[0]) + CountSetBits(tmp[1]);
	}
	static size_t CountSetBits(uint32_t v){
		const uint16_t *tmp = reinterpret_cast<const uint16_t *>(&v);
		return CountSetBits(tmp[0]) + CountSetBits(tmp[1]);
	}
	static size_t CountSetBits(uint64_t v){
		const uint32_t *tmp = reinterpret_cast<const uint32_t *>(&v);
		return CountSetBits(tmp[0]) + CountSetBits(tmp[1]);
	}
	static size_t CountSetBits(int8_t v){
		return HData::CountSetBits(static_cast<uint8_t>(v));
	}
	static size_t CountSetBits(int16_t v){
		return HData::CountSetBits(static_cast<uint16_t>(v));
	}
	static size_t CountSetBits(int32_t v){
		return HData::CountSetBits(static_cast<uint32_t>(v));
	}
	static size_t CountSetBits(int64_t v){
		return HData::CountSetBits(static_cast<uint64_t>(v));
	}
	static size_t CountSetBits(const void *data, size_t size){
		size_t count = 0;
		auto *d_ptr = &data;
		const uint8_t *u8_end = static_cast<const uint8_t *>(data)+size;

		for (const uint64_t **u = reinterpret_cast<const uint64_t **>(d_ptr); (*u + 1) <= reinterpret_cast<const uint64_t *>(u8_end); (*u)++){
			count += HData::CountSetBits(**u);
		}

		for (const uint32_t **u = reinterpret_cast<const uint32_t **>(d_ptr); (*u + 1) <= reinterpret_cast<const uint32_t *>(u8_end); (*u)++){
			count += HData::CountSetBits(**u);
		}

		for (const uint16_t **u = reinterpret_cast<const uint16_t **>(d_ptr); (*u + 1) <= reinterpret_cast<const uint16_t *>(u8_end); (*u)++){
			count += HData::CountSetBits(**u);
		}

		for (const uint8_t **u = reinterpret_cast<const uint8_t **>(d_ptr); *u < u8_end; (*u)++){
			count += HData::CountSetBits(**u);
		}

		return count;
	}

	static void HashRound(uint32_t &hash, uint32_t v){
		// http://stackoverflow.com/questions/2285822/c-what-is-a-good-way-to-hash-array-data
		hash = (hash + (324723947 + v)) ^ (uint32_t)93485734985;
	}

	static size_t CombineHash(size_t a, size_t b){
		// based on http://en.cppreference.com/w/cpp/utility/hash
		return a ^ (b << 1);
	}

	template<int intBits>
	static int ToFixedPoint(float v){
		const int fractBits = (sizeof(int) * 8) - intBits;
		const float scale = static_cast<float>(1 << fractBits);

		int res = static_cast<int>(v * scale);
		return res;
	}

	template<int intBits>
	static float FromFixedPoint(int v){
		const int fractBits = (sizeof(int) * 8) - intBits;
		const float scale = 1.0f / static_cast<float>(1 << fractBits);

		float res = static_cast<float>(v) * scale;
		return res;
	}

	template<class T>
	static T SwapBytes(const T &v){
		T dst;
		const uint8_t *srcPtr = reinterpret_cast<const uint8_t *>(&v);
		uint8_t *dstPtr = reinterpret_cast<uint8_t *>(&dst);

		for (size_t i = 0; i < sizeof(T); i++){
			dstPtr[i] = srcPtr[sizeof(T) - i - 1];
		}

		return dst;
	}

	template<>
	static uint64_t SwapBytes(const uint64_t &v){
#if HAVE_VISUAL_STUDIO == 1
		return _byteswap_uint64(v);
#else
		uint64_t dst = (v & 0xFF) << 56 | (v >> 8 & 0xFF) << 48 |
			(v >> 16 & 0xFF) << 40 | (v >> 24 & 0xFF) << 32 | 
			(v >> 32 & 0xFF) << 24 | (v >> 40 & 0xFF) << 16 |
			(v >> 48 & 0xFF) << 8 | (v >> 56 & 0xFF);
		return dst;
#endif
	}

	template<>
	static int64_t SwapBytes(const int64_t &v){
		return SwapBytes(static_cast<const uint64_t &>(v));
	}

	template<>
	static uint32_t SwapBytes(const uint32_t &v){
#if HAVE_VISUAL_STUDIO == 1
		return _byteswap_ulong(v);
#else
		uint32_t dst = (v & 0xFF) << 24 | (v >> 8 & 0xFF) << 16 | 
			(v >> 16 & 0xFF) << 8 | (v >> 24 & 0xFF);
		return dst;
#endif
	}

	template<>
	static int32_t SwapBytes(const int32_t &v){
		return SwapBytes(static_cast<const uint32_t &>(v));
	}

	template<>
	static uint16_t SwapBytes(const uint16_t &v){
#if HAVE_VISUAL_STUDIO == 1
		return _byteswap_ushort(v);
#else
		uint16_t dst = (v & 0xFF) << 8 | (v >> 8 & 0xFF);
		return dst;
#endif
	}

	template<>
	static int16_t SwapBytes(const int16_t &v){
		return SwapBytes(static_cast<const uint16_t &>(v));
	}

	template<>
	static uint8_t SwapBytes(const uint8_t &v){
		return v;
	}

	template<>
	static int8_t SwapBytes(const int8_t &v){
		return v;
	}
};