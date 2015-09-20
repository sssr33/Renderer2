#pragma once
#include "config.h"

#ifdef HAVE_VISUAL_STUDIO
#include <intrin.h>
#include <windows.h>
#endif

#include <cstdint>
#include <chrono>
#include <ctime>

class HTime{
	// hundred nano-seconds resolution. HNS in 1 second
public:
	static const uint64_t HNSResolution = 10000000ULL;

private:
	// HNS in 1 minute
	static const uint64_t MinuteTicks = HNSResolution * 60ULL;
	// HNS in 1 hour
	static const uint64_t HourTicks = MinuteTicks * 60ULL;
	// HNS in 1 day
	static const uint64_t DayTicks = HourTicks * 24ULL;
	// HNS in 1 year
	// 365.2425 - http://en.wikipedia.org/wiki/Gregorian_calendar
	static const uint64_t YearTicks = HNSResolution * static_cast<uint64_t>(60.0 * 60.0 * 24.0 * 365.2425);
	static const uint64_t Y1970FullTicks = YearTicks * 1970ULL;
	static const uint64_t Y1601FullTicks = YearTicks * 1601ULL;
	// how much time taken by leap days
	static const uint64_t Leap1970Ticks = Y1970FullTicks % DayTicks;
	static const uint64_t Leap1601Ticks = Y1601FullTicks % DayTicks;
	static const uint64_t Y1970Ticks = Y1970FullTicks - Leap1970Ticks;
	static const uint64_t Y1601Ticks = Y1601FullTicks - Leap1601Ticks;
public:	
	
	// Returns number of milliseconds since some event(for example since the system was started).
	static uint64_t GetTimeMsRelative(){
#if defined(HAVE_VISUAL_STUDIO)
		return GetTickCount64();
#else
#error Add impl
#endif
	}

	// Converts Unix time(seconds since 1970 year) to Library time(hundred nano-seconds(i.e. 1/10,000,000 of second) since 0 year).
	static uint64_t UnixTimeToLib(uint64_t unixTime){
		uint64_t libTime = HTime::Y1970Ticks + (unixTime * HTime::HNSResolution);
		return libTime;
	}

	// Converts Library time(hundred nano-seconds(i.e. 1/10,000,000 of second) since 0 year) to Unix time(seconds since 1970 year).
	static uint64_t LibTimeToUnix(uint64_t libTime){
		uint64_t unixTime = (libTime - HTime::Y1970Ticks) / HTime::HNSResolution;
		return unixTime;
	}

	static uint64_t GetCurrentLibTime(){
		typedef std::ratio<1, static_cast<int64_t>(HTime::HNSResolution)> hns;
		typedef std::chrono::duration<int64_t, hns> hundred_nanoseconds;
		uint64_t libTime = 0;
		auto nowTp = std::chrono::system_clock::now();
		uint64_t nowSysHns = static_cast<uint64_t>(std::chrono::duration_cast<hundred_nanoseconds>(nowTp.time_since_epoch()).count());

		libTime = HTime::GetSysClockEpochTicks() + nowSysHns;

		return libTime;
	}

	static uint64_t CeilLibTimeToSeconds(uint64_t libTime){
		uint64_t millis = libTime % HTime::HNSResolution;

		libTime -= millis;

		if (millis != 0){
			libTime += HTime::HNSResolution;
		}

		return libTime;
	}

#ifdef HAVE_VISUAL_STUDIO
	static uint64_t FileTimeToLib(FILETIME fileTime){
		uint64_t tmpFileTime = static_cast<uint64_t>(fileTime.dwLowDateTime) | (static_cast<uint64_t>(fileTime.dwHighDateTime) << 32);
		uint64_t libTime = HTime::Y1601Ticks + tmpFileTime;
		return libTime;
	}

	static FILETIME LibTimeToFile(uint64_t libTime){
		uint64_t tmpFileTime = libTime - HTime::Y1601Ticks;
		FILETIME fileTime;

		fileTime.dwLowDateTime = static_cast<DWORD>(tmpFileTime & 0x00000000FFFFFFFF);
		fileTime.dwHighDateTime = static_cast<DWORD>((tmpFileTime >> 32) & 0x00000000FFFFFFFF);

		return fileTime;
	}

	// based on http://stackoverflow.com/questions/11615998/how-do-i-parse-a-date-in-a-metro-c-cx-app
	static uint64_t UniversalTimeToLib(int64_t universalTime){
		uint64_t libTime = static_cast<uint64_t>(static_cast<int64_t>(HTime::Y1601Ticks) + universalTime);
		return libTime;
	}

	static int64_t LibTimeToUniversal(uint64_t libTime){
		int64_t universalTime = static_cast<int64_t>(libTime - HTime::Y1601Ticks);
		return universalTime;
	}

	// returns localLibTime
	static uint64_t LibTimeToLocal(uint64_t libTime){
		uint64_t localLibTime;
		SYSTEMTIME st, st_utc;
		auto ft = HTime::LibTimeToFile(libTime);

		FileTimeToSystemTime(&ft, &st_utc);
		SystemTimeToTzSpecificLocalTime(nullptr, &st_utc, &st);
		SystemTimeToFileTime(&st, &ft);

		localLibTime = HTime::FileTimeToLib(ft);

		return localLibTime;
	}

	// returns libTime
	static uint64_t LibTimeToGlobal(uint64_t localLibTime){
		uint64_t libTime;
		SYSTEMTIME st, st_utc;
		auto ft = HTime::LibTimeToFile(localLibTime);

		FileTimeToSystemTime(&ft, &st);
		TzSpecificLocalTimeToSystemTime(nullptr, &st, &st_utc);
		SystemTimeToFileTime(&st_utc, &ft);

		libTime = HTime::FileTimeToLib(ft);

		return libTime;
	}

#endif
private:
	static uint64_t GetSysClockEpochTicks(){
		static uint64_t SysClockEpochTicks = 0;

		if (SysClockEpochTicks == 0){
			std::chrono::system_clock::time_point epochTp;
			tm epochTm;
			auto epochTimeT = std::chrono::system_clock::to_time_t(epochTp);
			gmtime_s(&epochTm, &epochTimeT);
			uint64_t epochFullTicks = HTime::YearTicks * (epochTm.tm_year + 1900);
			uint64_t epochLeapTicks = epochFullTicks % HTime::DayTicks;
			epochFullTicks -= epochLeapTicks;

			epochFullTicks += HTime::DayTicks * epochTm.tm_yday;
			epochFullTicks += HTime::HourTicks * epochTm.tm_hour;
			epochFullTicks += HTime::MinuteTicks * epochTm.tm_min;
			epochFullTicks += HTime::HNSResolution * epochTm.tm_sec;

			SysClockEpochTicks = epochFullTicks;
		}

		return SysClockEpochTicks;
	}
};