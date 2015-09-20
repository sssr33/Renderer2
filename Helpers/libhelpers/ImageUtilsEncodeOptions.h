#pragma once
#include "H.h"

#include <wincodec.h>

template<class Impl>
class ImageUtilsEncodeOptions{
public:
	HRESULT Save(IPropertyBag2 *props){
		return this->SaveImpl(props);
	}
protected:
	ImageUtilsEncodeOptions(){}
	~ImageUtilsEncodeOptions(){}

	HRESULT SaveImpl(IPropertyBag2 *props){
		return static_cast<Impl *>(this)->SaveImpl(props);
	}
};

class ImageUtilsEncodeNoOptions : public ImageUtilsEncodeOptions < ImageUtilsEncodeNoOptions > {
public:
	ImageUtilsEncodeNoOptions(){
	}
	HRESULT SaveImpl(IPropertyBag2 *props){
		return S_OK;
	}
};

class ImageUtilsEncodeJpegOptions : public ImageUtilsEncodeOptions < ImageUtilsEncodeJpegOptions > {
public:
	float Quality;

	ImageUtilsEncodeJpegOptions()
		: Quality(1.0f){
	}
	HRESULT SaveImpl(IPropertyBag2 *props){
		PROPBAG2 option = { 0 };
		option.pstrName = L"ImageQuality";
		VARIANT varValue;
		varValue.vt = VT_R4;
		varValue.fltVal = this->Quality;

		return props->Write(1, &option, &varValue);
	}
};