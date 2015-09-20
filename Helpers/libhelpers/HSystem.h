#pragma once
#include "config.h"
#include "HData.h"

#include <cstdint>
#include <string>
#include <future>
#include <memory>
#include <functional>
#include <assert.h>
#include <exception>

#ifdef HAVE_VISUAL_STUDIO
#include <intrin.h>
#include <windows.h>
#endif

class HSystem{
public:
	static void DebuggerBreak(){
#if defined(HAVE_VISUAL_STUDIO) && defined(_DEBUG)
		__debugbreak();
#else
		// Just place breakpoint on this line:
		int stop = 234;
#endif
	}
	static void Assert(bool expression){
		if (!expression){
			HSystem::DebuggerBreak();
		}
		assert(expression);
	}
	static void DebugOutput(const std::string &v){
#ifdef _DEBUG
#ifdef HAVE_VISUAL_STUDIO
		OutputDebugStringA("\n");
		OutputDebugStringA(v.c_str());
#else
		printf("\n%s", v.c_str());
#endif
#endif
	}
	static std::wstring FixPathWinRT(const std::wstring &v){
		std::wstring fixed;

		if (!v.empty()){
			size_t startIdx = 0;
			size_t size = v.size();

			if (v[startIdx] == '\\' || v[startIdx] == '/'){
				startIdx++;
			}

			if (v[size - 1] == '\\' || v[size - 1] == '/'){
				size--;
			}

			fixed.reserve(size);

			for (size_t i = startIdx; i < size; i++){
				if (v[i] != '/'){
					fixed.push_back(v[i]);
				}
				else{
					fixed.push_back('\\');
				}
			}
		}

		return fixed;
	}
	static void ThrowIfFailed(HRESULT hr){
#if HAVE_WINRT == 1
		if (FAILED(hr)){
			throw ref new Platform::Exception(hr);
		}
#else
		if (FAILED(hr)){
			throw std::exception();
		}
#endif
	}

	static std::vector<uint8_t> LoadPackageFile(const std::wstring &path){
		std::vector<uint8_t> fileData;
		std::wstring fullPath = HSystem::GetPackagePath() + path;
		auto file = CreateFile2(fullPath.c_str(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, nullptr);

		if (file != INVALID_HANDLE_VALUE){
			DWORD readed;
			LARGE_INTEGER pos, newPos, fileSize;

			pos.QuadPart = 0;
			newPos.QuadPart = 0;

			SetFilePointerEx(file, pos, &newPos, FILE_END);
			fileSize = newPos;
			SetFilePointerEx(file, pos, &newPos, FILE_BEGIN);

			fileData.resize(static_cast<size_t>(fileSize.QuadPart));

			auto res = ReadFile(file, fileData.data(), (DWORD)fileData.size(), &readed, nullptr);

			CloseHandle(file);
		}

		return fileData;
	}
	static std::wstring GetPackagePath(){
		std::wstring packagePath;
#if HAVE_WINRT == 1
		auto tmpPath = Windows::ApplicationModel::Package::Current->InstalledLocation->Path;
		std::wstring installedPath(tmpPath->Data(), tmpPath->Length());
		packagePath = installedPath + L"\\";
#else
		WCHAR exePath[MAX_PATH];
		GetModuleFileNameW(nullptr, exePath, MAX_PATH);
		packagePath = exePath;
		packagePath = packagePath.substr(0, packagePath.find_last_of('\\') + 1);
#endif
		return packagePath;
	}

	static ULONG GetRefCount(IUnknown *v) {
		if (!v) {
			return 0;
		}

		v->AddRef();
		return v->Release();
	}

#if HAVE_WINRT == 1
	static HRESULT PerformSync(Windows::Foundation::IAsyncAction ^op){
		std::promise<HRESULT> prom;
		auto fut = prom.get_future();

		op->Completed = ref new Windows::Foundation::AsyncActionCompletedHandler(
			[&](Windows::Foundation::IAsyncAction ^op, Windows::Foundation::AsyncStatus status){
			HRESULT res;

			if (status == Windows::Foundation::AsyncStatus::Completed){
				res = S_OK;
			}
			else{
				res = static_cast<HRESULT>(op->ErrorCode.Value);
			}

			prom.set_value(res);
		});

		auto result = fut.get();

		return result;
	}
	static void PerformSyncThrow(Windows::Foundation::IAsyncAction ^op){
		HSystem::ThrowIfFailed(HSystem::PerformSync(op));
	}

	template<typename T>
	static HRESULT PerformSync(Windows::Foundation::IAsyncActionWithProgress<T> ^op){
		std::promise<HRESULT> prom;
		auto fut = prom.get_future();

		op->Completed = ref new Windows::Foundation::AsyncActionWithProgressCompletedHandler<T>(
			[&](Windows::Foundation::IAsyncActionWithProgress<T> ^op, Windows::Foundation::AsyncStatus status){
			HRESULT res;

			if (status == Windows::Foundation::AsyncStatus::Completed){
				res = S_OK;
			}
			else{
				res = static_cast<HRESULT>(op->ErrorCode.Value);
			}

			prom.set_value(res);
		});

		auto result = fut.get();

		return result;
	}

	template<typename T>
	static void PerformSyncThrow(Windows::Foundation::IAsyncActionWithProgress<T> ^op){
		HSystem::ThrowIfFailed(HSystem::PerformSync(op));
	}

	template<typename T>
	static std::pair<HRESULT, T> PerformSync(Windows::Foundation::IAsyncOperation<T> ^op){
		std::promise<std::pair<HRESULT, T>> prom;	//break exception there
		auto fut = prom.get_future();

		op->Completed = ref new Windows::Foundation::AsyncOperationCompletedHandler<T>(
			[&](Windows::Foundation::IAsyncOperation<T> ^op, Windows::Foundation::AsyncStatus status){
			std::pair<HRESULT, T> res;

			if (status == Windows::Foundation::AsyncStatus::Completed){
				res.first = S_OK;
				res.second = op->GetResults();
			}
			else{
				res.first = static_cast<HRESULT>(op->ErrorCode.Value);
				res.second = T();
			}

			prom.set_value(res);
		});

		auto result = fut.get();

		return result;
	}

	template<typename T>
	static T PerformSyncThrow(Windows::Foundation::IAsyncOperation<T> ^op){
		auto res = HSystem::PerformSync(op);
		HSystem::ThrowIfFailed(res.first);
		return res.second;
	}

	template<typename T, typename T2>
	static std::pair<HRESULT, T> PerformSync(Windows::Foundation::IAsyncOperationWithProgress<T, T2> ^op){
		std::promise<std::pair<HRESULT, T>> prom;
		auto fut = prom.get_future();

		op->Completed = ref new Windows::Foundation::AsyncOperationWithProgressCompletedHandler<T, T2>(
			[&](Windows::Foundation::IAsyncOperationWithProgress<T, T2> ^op, Windows::Foundation::AsyncStatus status){
			std::pair<HRESULT, T> res;

			if (status == Windows::Foundation::AsyncStatus::Completed){
				res.first = S_OK;
				res.second = op->GetResults();
			}
			else{
				res.first = static_cast<HRESULT>(op->ErrorCode.Value);
				res.second = T();
			}

			prom.set_value(res);
		});

		auto result = fut.get();

		return result;
	}

	template<typename T, typename T2>
	static T PerformSyncThrow(Windows::Foundation::IAsyncOperationWithProgress<T, T2> ^op){
		auto res = HSystem::PerformSync(op);
		HSystem::ThrowIfFailed(res.first);
		return res.second;
	}
#endif
};

#ifdef HAVE_VISUAL_STUDIO

struct GUIDHash{
	std::size_t operator()(const GUID &v) const{
		const uint64_t *tmpData4 = reinterpret_cast<const uint64_t *>(v.Data4);

		std::size_t h1 = std::hash<unsigned long>()(v.Data1);
		std::size_t h2 = std::hash<unsigned short>()(v.Data2);
		std::size_t h3 = std::hash<unsigned short>()(v.Data3);
		std::size_t h4 = std::hash<uint64_t>()(*tmpData4);

		return HData::CombineHash(h1, HData::CombineHash(h2, HData::CombineHash(h3, h4)));
	}
};

#endif