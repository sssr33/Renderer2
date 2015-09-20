#pragma once
#include "PixelConverter.h"

#include <cstdint>

template<bool enabled, uint8_t defaultValue = 255>
class PixelComponentValueGetter{
public:
	static const bool Enabled = enabled;

	static uint8_t Get(const uint8_t *ptr){
		return defaultValue;
	}
};

template<bool enabled, int byteOffset, uint8_t defaultValue = 255>
class PixelComponentGetter{
public:
	static const bool Enabled = enabled;

	static uint8_t Get(const uint8_t *ptr){
		return ptr[byteOffset];
	}
};

template<int byteOffset, uint8_t defaultValue>
class PixelComponentGetter < false, byteOffset, defaultValue > {
public:
	static const bool Enabled = false;

	static uint8_t Get(const uint8_t *ptr){
		return defaultValue;
	}
};

template<bool enabled, int byteOffset>
class PixelComponentSetter{
public:
	static const bool Enabled = enabled;

	static void Set(uint8_t *ptr, uint8_t v){
		ptr[byteOffset] = v;
	}
};

template<int byteOffset>
class PixelComponentSetter < false, byteOffset > {
public:
	static const bool Enabled = false;

	static void Set(uint8_t *ptr, uint8_t v){
	}
};




template<bool v>
class BoolToInt{
public:
	static const int Value = 1;
};

template<>
class BoolToInt < false > {
public:
	static const int Value = 0;
};

template<int v, int add>
class Accumulator{
public:
	static const int Value = v + add;
};




template<class R, class G, class B, class A>
class PixelGetter{
	typedef BoolToInt<R::Enabled> AccumArgR;
	typedef BoolToInt<G::Enabled> AccumArgG;
	typedef BoolToInt<B::Enabled> AccumArgB;
	typedef BoolToInt<A::Enabled> AccumArgA;
public:
	static const int PixelByteSize =
		Accumulator <
		Accumulator<
		Accumulator <
		Accumulator<0, AccumArgR::Value>::Value,
		AccumArgG::Value > ::Value,
		AccumArgB::Value>::Value,
		AccumArgA::Value > ::Value;

	static uint8_t GetR(const uint8_t *ptr){
		return R::Get(ptr);
	}

	static uint8_t GetG(const uint8_t *ptr){
		return G::Get(ptr);
	}

	static uint8_t GetB(const uint8_t *ptr){
		return B::Get(ptr);
	}

	static uint8_t GetA(const uint8_t *ptr){
		return A::Get(ptr);
	}
};

template<class R, class G, class B, class A>
class PixelSetter{
	typedef BoolToInt<R::Enabled> AccumArgR;
	typedef BoolToInt<G::Enabled> AccumArgG;
	typedef BoolToInt<B::Enabled> AccumArgB;
	typedef BoolToInt<A::Enabled> AccumArgA;
public:
	static const int PixelByteSize =
		Accumulator <
		Accumulator<
		Accumulator <
		Accumulator<0, AccumArgR::Value>::Value,
		AccumArgG::Value > ::Value,
		AccumArgB::Value>::Value,
		AccumArgA::Value > ::Value;

	static void SetR(uint8_t *ptr, uint8_t v){
		R::Set(ptr, v);
	}

	static void SetG(uint8_t *ptr, uint8_t v){
		G::Set(ptr, v);
	}

	static void SetB(uint8_t *ptr, uint8_t v){
		B::Set(ptr, v);
	}

	static void SetA(uint8_t *ptr, uint8_t v){
		A::Set(ptr, v);
	}
};




template<class Setter, class Getter>
class PixelConverterStd8Bit : public PixelConverter{
public:
	PixelConverterStd8Bit(){
	}

	virtual ~PixelConverterStd8Bit(){
	}

	virtual void Convert(void *dst, const void *src, uint32_t pixelCount) override{
		uint8_t *dst8Bit = static_cast<uint8_t *>(dst);
		const uint8_t *src8Bit = static_cast<const uint8_t *>(src);

		for (uint32_t i = 0; i < pixelCount; i++, dst8Bit += Setter::PixelByteSize, src8Bit += Getter::PixelByteSize){
			auto r = Getter::GetR(src8Bit);
			auto g = Getter::GetG(src8Bit);
			auto b = Getter::GetB(src8Bit);
			auto a = Getter::GetA(src8Bit);

			Setter::SetR(dst8Bit, r);
			Setter::SetG(dst8Bit, g);
			Setter::SetB(dst8Bit, b);
			Setter::SetA(dst8Bit, a);
		}
	}
};