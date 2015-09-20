#pragma once
#include "..\H.h"
#include "PixelConverter.h"
#include "PixelConverterCopy.h"
#include "PixelConverterStd8Bit.h"
#include "..\ImageUtils.h"

#include <unordered_map>
#include <functional>
#include <memory>

class PixelFormatConversionDesc{
public:
	GUID SourceFormat;
	GUID DestinationFormat;

	bool operator==(const PixelFormatConversionDesc &other) const{
		bool res =
			IsEqualGUID(this->SourceFormat, other.SourceFormat) &&
			IsEqualGUID(this->DestinationFormat, other.DestinationFormat);
		return res;
	}
};

struct PixelFormatConversionDescHash{
	std::size_t operator()(const PixelFormatConversionDesc &v) const{
		GUIDHash hashFn;

		auto srcHash = hashFn(v.SourceFormat);
		auto dstHash = hashFn(v.DestinationFormat);

		return H::Data::CombineHash(srcHash, dstHash);
	}
};

class PixelConverterFactory{
	static std::unordered_map <
		PixelFormatConversionDesc,
		std::function<PixelConverter *()>,
		PixelFormatConversionDescHash > creators;
	static int StaticCtorRes;
	static int StaticCtor(){
		PixelConverterFactory::InitCreators();

		return 0;
	}
	static void InitCreators(){
		typedef PixelComponentGetter<true, 0> GetR;
		typedef PixelComponentGetter<true, 1> GetG;
		typedef PixelComponentGetter<true, 2> GetB;
		typedef PixelComponentGetter<true, 3> GetA;
		typedef PixelComponentGetter<false, 3> GetADisabled;

		typedef PixelComponentSetter<true, 0> SetR;
		typedef PixelComponentSetter<true, 1> SetG;
		typedef PixelComponentSetter<true, 2> SetB;
		typedef PixelComponentSetter<true, 3> SetA;
		typedef PixelComponentSetter<false, 3> SetADisabled;

		PixelFormatConversionDesc convDesc;

		// 32 -->> 24
		convDesc.SourceFormat = GUID_WICPixelFormat32bppBGRA;
		convDesc.DestinationFormat = GUID_WICPixelFormat24bppRGB;
		PixelConverterFactory::creators.insert(std::make_pair(convDesc, [](){
			typedef PixelGetter<GetB, GetG, GetR, GetA> Getter;
			typedef PixelSetter<SetR, SetG, SetB, SetADisabled> Setter;

			return new PixelConverterStd8Bit < Setter, Getter > ;
		}));

		convDesc.SourceFormat = GUID_WICPixelFormat32bppRGBA;
		convDesc.DestinationFormat = GUID_WICPixelFormat24bppRGB;
		PixelConverterFactory::creators.insert(std::make_pair(convDesc, [](){
			typedef PixelGetter<GetR, GetG, GetB, GetA> Getter;
			typedef PixelSetter<SetR, SetG, SetB, SetADisabled> Setter;

			return new PixelConverterStd8Bit < Setter, Getter > ;
		}));

		convDesc.SourceFormat = GUID_WICPixelFormat32bppBGRA;
		convDesc.DestinationFormat = GUID_WICPixelFormat24bppBGR;
		PixelConverterFactory::creators.insert(std::make_pair(convDesc, [](){
			typedef PixelGetter<GetB, GetG, GetR, GetA> Getter;
			typedef PixelSetter<SetB, SetG, SetR, SetADisabled> Setter;

			return new PixelConverterStd8Bit < Setter, Getter > ;
		}));

		convDesc.SourceFormat = GUID_WICPixelFormat32bppRGBA;
		convDesc.DestinationFormat = GUID_WICPixelFormat24bppBGR;
		PixelConverterFactory::creators.insert(std::make_pair(convDesc, [](){
			typedef PixelGetter<GetR, GetG, GetB, GetA> Getter;
			typedef PixelSetter<SetB, SetG, SetR, SetADisabled> Setter;

			return new PixelConverterStd8Bit < Setter, Getter > ;
		}));

		// 24 -->> 32
		convDesc.SourceFormat = GUID_WICPixelFormat24bppRGB;
		convDesc.DestinationFormat = GUID_WICPixelFormat32bppBGRA;
		PixelConverterFactory::creators.insert(std::make_pair(convDesc, [](){
			typedef PixelGetter<GetR, GetG, GetB, GetADisabled> Getter;
			typedef PixelSetter<SetB, SetG, SetR, SetA> Setter;

			return new PixelConverterStd8Bit < Setter, Getter > ;
		}));

		convDesc.SourceFormat = GUID_WICPixelFormat24bppRGB;
		convDesc.DestinationFormat = GUID_WICPixelFormat32bppRGBA;
		PixelConverterFactory::creators.insert(std::make_pair(convDesc, [](){
			typedef PixelGetter<GetR, GetG, GetB, GetADisabled> Getter;
			typedef PixelSetter<SetR, SetG, SetB, SetA> Setter;

			return new PixelConverterStd8Bit < Setter, Getter > ;
		}));

		convDesc.SourceFormat = GUID_WICPixelFormat24bppBGR;
		convDesc.DestinationFormat = GUID_WICPixelFormat32bppBGRA;
		PixelConverterFactory::creators.insert(std::make_pair(convDesc, [](){
			typedef PixelGetter<GetB, GetG, GetR, GetADisabled> Getter;
			typedef PixelSetter<SetB, SetG, SetR, SetA> Setter;

			return new PixelConverterStd8Bit < Setter, Getter > ;
		}));

		convDesc.SourceFormat = GUID_WICPixelFormat24bppBGR;
		convDesc.DestinationFormat = GUID_WICPixelFormat32bppRGBA;
		PixelConverterFactory::creators.insert(std::make_pair(convDesc, [](){
			typedef PixelGetter<GetB, GetG, GetR, GetADisabled> Getter;
			typedef PixelSetter<SetR, SetG, SetB, SetA> Setter;

			return new PixelConverterStd8Bit < Setter, Getter > ;
		}));

		// 32 -->> 32
		convDesc.SourceFormat = GUID_WICPixelFormat32bppRGBA;
		convDesc.DestinationFormat = GUID_WICPixelFormat32bppBGRA;
		PixelConverterFactory::creators.insert(std::make_pair(convDesc, [](){
			typedef PixelGetter<GetR, GetG, GetB, GetA> Getter;
			typedef PixelSetter<SetB, SetG, SetR, SetA> Setter;

			return new PixelConverterStd8Bit < Setter, Getter > ;
		}));

		convDesc.SourceFormat = GUID_WICPixelFormat32bppBGRA;
		convDesc.DestinationFormat = GUID_WICPixelFormat32bppRGBA;
		PixelConverterFactory::creators.insert(std::make_pair(convDesc, [](){
			typedef PixelGetter<GetB, GetG, GetR, GetA> Getter;
			typedef PixelSetter<SetR, SetG, SetB, SetA> Setter;

			return new PixelConverterStd8Bit < Setter, Getter > ;
		}));
	}
public:
	static PixelConverter *CreateConverter(const GUID &src, const GUID &dst){
		PixelConverter *res = nullptr;
		PixelFormatConversionDesc convDesc;

		if (IsEqualGUID(src, dst)){
			ImageUtils tmp;
			uint32_t pixelByteSize = tmp.GetPixelBitSize(src) / 8;
			res = new PixelConverterCopy(pixelByteSize);
		}
		else{
			convDesc.SourceFormat = src;
			convDesc.DestinationFormat = dst;

			auto finded = PixelConverterFactory::creators.find(convDesc);
			if (finded != PixelConverterFactory::creators.end()){
				res = finded->second();
			}
		}

		return res;
	}
};

std::unordered_map <
	PixelFormatConversionDesc,
	std::function<PixelConverter *()>,
	PixelFormatConversionDescHash > PixelConverterFactory::creators;
int PixelConverterFactory::StaticCtorRes = PixelConverterFactory::StaticCtor();