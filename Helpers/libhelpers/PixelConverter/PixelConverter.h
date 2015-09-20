#pragma once

#include <cstdint>

class PixelConverter{
public:
	PixelConverter();
	virtual ~PixelConverter();

	virtual void Convert(void *dst, const void *src, uint32_t pixelCount) = 0;
};