#pragma once
#include "config.h"
#include "HText.h"
#include "HData.h"
#include "HSystem.h"
#include "HTime.h"
#include "HMath.h"
#include "Macros.h"

#include "WinRT/HDataWinRT.h"

class H{
public:
	typedef HText Text;
	typedef HData Data;
	typedef HSystem System;
	typedef HTime Time;
	typedef HMath Math;

#if HAVE_WINRT == 1
	class WinRT{
	public:
		typedef HDataWinRT Data;
	};
#endif
};