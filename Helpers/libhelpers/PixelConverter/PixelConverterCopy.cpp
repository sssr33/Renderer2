#if INCLUDE_PCH_H == 1
#include "pch.h"
#endif
#include "PixelConverterCopy.h"

#include <string>

PixelConverterCopy::PixelConverterCopy(uint32_t pixelByteSize)
	: pixelByteSize(pixelByteSize){
}

PixelConverterCopy::~PixelConverterCopy(){
}

void PixelConverterCopy::Convert(void *dst, const void *src, uint32_t pixelCount){
	std::memcpy(dst, src, this->pixelByteSize * pixelCount);
}