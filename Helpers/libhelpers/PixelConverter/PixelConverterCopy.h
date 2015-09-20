#pragma once
#include "PixelConverter.h"

class PixelConverterCopy : public PixelConverter{
public:
	PixelConverterCopy(uint32_t pixelByteSize);
	virtual ~PixelConverterCopy();

	virtual void Convert(void *dst, const void *src, uint32_t pixelCount) override;
private:
	uint32_t pixelByteSize;
};