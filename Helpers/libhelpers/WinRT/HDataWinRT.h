#pragma once
#include "..\HData.h"

#if HAVE_WINRT == 1
class HDataWinRT{
public:

	static Windows::Data::Json::JsonArray ^SavePoint(float x, float y){
		Windows::Data::Json::JsonArray ^jsonPoint = ref new Windows::Data::Json:: JsonArray;

		jsonPoint->Append(Windows::Data::Json::JsonValue::CreateNumberValue(x));
		jsonPoint->Append(Windows::Data::Json::JsonValue::CreateNumberValue(y));

		return jsonPoint;
	}

	static void LoadPoint(Windows::Data::Json::JsonArray ^json, float *x, float *y){
		if (x){
			*x = static_cast<float>(json->GetNumberAt(0));
		}

		if (y){
			*y = static_cast<float>(json->GetNumberAt(1));
		}
	}

	template<int intBits>
	static Windows::Data::Json::JsonArray ^SavePointFixed(float x, float y){
		Windows::Data::Json::JsonArray ^jsonPoint = ref new Windows::Data::Json::JsonArray;
		int pt;
		const wchar_t *ptWChars = reinterpret_cast<const wchar_t *>(&pt);

		// need to save 2 wchar_t because sizeof(wchar_t) == 2 and sizeof(int) == 4

		pt = H::Data::ToFixedPoint<intBits>(x);
		jsonPoint->Append(Windows::Data::Json::JsonValue::CreateStringValue(
			ref new Platform::String(ptWChars, 2)));

		pt = H::Data::ToFixedPoint<intBits>(y);
		jsonPoint->Append(Windows::Data::Json::JsonValue::CreateStringValue(
			ref new Platform::String(ptWChars, 2)));

		return jsonPoint;
	}

	template<int intBits>
	static void LoadPointFixed(Windows::Data::Json::JsonArray ^json, float *x, float *y){
		Platform::String ^str;
		const int *fixPt;

		if (x){
			str = jsonPoint->GetStringAt(0);
			fixPt = reinterpret_cast<const int *>(str->Data());
			*x = H::Data::FromFixedPoint<intBits>(*fixPt);
		}

		if (y){
			str = jsonPoint->GetStringAt(1);
			fixPt = reinterpret_cast<const int *>(str->Data());
			*y = H::Data::FromFixedPoint<intBits>(*fixPt);
		}
	}

};
#endif