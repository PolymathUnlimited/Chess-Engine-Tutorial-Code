#pragma once

#ifdef _MSC_VER
#include <intrin.h>
#endif
#include <cstdint>
#include <cstring>

inline std::uint8_t lsbIndex(std::uint64_t x)
{
#if defined(_MSC_VER) // visual studio version
	unsigned long index;
	_BitScanForward64(&index, x);
	return index;
#elif defined(__GNUC__) || defined(__clang__) // clang/gcc version
	return __builtin_ctzll(x);
#else // fallback
	static const int index[64] =
	{
		 0,  1, 48,  2, 57, 49, 28,  3,
		61, 58, 50, 42, 38, 29, 17,  4,
		62, 55, 59, 36, 53, 51, 43, 22,
		45, 39, 33, 30, 24, 18, 12,  5,
		63, 47, 56, 27, 60, 41, 37, 16,
		54, 35, 52, 21, 44, 32, 23, 11,
		46, 26, 40, 15, 34, 20, 31, 10,
		25, 14, 19,  9, 13,  8,  7,  6
	};
	static const std::uint64_t debruijn = 0x03f79d7b4cb0a89;
	return index[((x & -(std::int64_t)x) * debruijn) >> 58];
#endif
}

inline std::uint8_t msbIndex(std::uint64_t x)
{
#if defined(_MSC_VER) // visual studio version
	unsigned long index;
	_BitScanReverse64(&index, x);
	return index;
#elif defined(__GNUC__) || deifned(__clang__) // clang/gcc version
	return 63 - __builtin_clzll(x);
#else // fallback
	double d = (double)x;
	std::uint64_t bits;
	memcpy(&bits, &d, sizeof(bits));
	return ((bits >> 52) & 0x7ff) - 1023;
#endif
}

inline std::uint8_t popcount(uint64_t x)
{
#if defined(_MSC_VER) // visual studio version
	return __popcnt64(x);
#elif defined(__GNUC__) || defined(__clang__) // clang/gcc version
	return __builtin_popcountll(x);
#else // fallback
	int count = 0;
	while (x)
	{
		x &= x - 1;
		++count;
	}
	return count;
#endif
}