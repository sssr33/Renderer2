#pragma once
#include "config.h"

#if HAVE_WINRT == 1

#include <Windows.h>
#include <winstring.h>

class PlatformStringBuilder{
public:
	PlatformStringBuilder()
		: string(nullptr), stringBuffer(nullptr), strBuffer(nullptr), length(0) {
	}

	~PlatformStringBuilder() {
		this->Release();
	}

	uint32_t GetLength() const {
		return this->length;
	}

	wchar_t *GetData() {
		return this->strBuffer;
	}

	void Resize(uint32_t length) {
		this->Release();

		HRESULT hr = WindowsPreallocateStringBuffer(length, &this->strBuffer, &this->stringBuffer);

		if (FAILED(hr)) {
			this->strBuffer = nullptr;
			this->stringBuffer = nullptr;
		}
		else {
			this->length = length;
		}
	}

	Platform::String ^Create() {
		Platform::String ^str = nullptr;

		if (this->stringBuffer) {
			HRESULT hr = WindowsPromoteStringBuffer(this->stringBuffer, &this->string);

			if (SUCCEEDED(hr)) {
				// stringBuffer owned by string
				this->stringBuffer = nullptr;
				
				str = ref new Platform::String(this->string);
			}
			else {
				this->string = nullptr;
			}

			this->Release();
		}

		return str;
	}

private:
	HSTRING string;
	HSTRING_BUFFER stringBuffer;
	PWSTR strBuffer;
	uint32_t length;

	void Release() {
		if (this->string) {
			WindowsDeleteString(this->string);
			this->string = nullptr;
		}

		if (this->stringBuffer) {
			WindowsDeleteStringBuffer(this->stringBuffer);
			this->stringBuffer = nullptr;
			this->strBuffer = nullptr;
		}

		this->length = 0;
	}
};

#endif