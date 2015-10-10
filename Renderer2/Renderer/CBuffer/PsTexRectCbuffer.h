#pragma once

#include <DirectXMath.h>

struct PsTexRectCbuffer {
	DirectX::XMFLOAT4X4 ColorMtrx;
	DirectX::XMFLOAT4 ColorAdd;
};