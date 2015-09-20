#pragma once
#include "config.h"
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <d2d1.h>

// Defines for accessing XMVECTOR components without XMVectorGet.. , XMVectorSet.. methods
#ifdef _M_ARM

#define XF n128_f32[0]
#define YF n128_f32[1]
#define ZF n128_f32[2]
#define WF n128_f32[3]

#define XU32 n128_u32[0]
#define YU32 n128_u32[1]
#define ZU32 n128_u32[2]
#define WU32 n128_u32[3]

#define XI32 n128_i32[0]
#define YI32 n128_i32[1]
#define ZI32 n128_i32[2]
#define WI32 n128_i32[3]

#define F32 n128_f32

#define I8 n128_i8
#define I16 n128_i16
#define I32 n128_i32
#define I64 n128_i64

#define U8 n128_u8
#define U16 n128_u16
#define U32 n128_u32
#define U64 n128_u64

#else

#define XF m128_f32[0]
#define YF m128_f32[1]
#define ZF m128_f32[2]
#define WF m128_f32[3]

#define XU32 m128_u32[0]
#define YU32 m128_u32[1]
#define ZU32 m128_u32[2]
#define WU32 m128_u32[3]

#define XI32 m128_i32[0]
#define YI32 m128_i32[1]
#define ZI32 m128_i32[2]
#define WI32 m128_i32[3]

#define F32 m128_f32

#define I8 m128_i8
#define I16 m128_i16
#define I32 m128_i32
#define I64 m128_i64

#define U8 m128_u8
#define U16 m128_u16
#define U32 m128_u32
#define U64 m128_u64

#endif

class HMath{
public:

	/// <summary>
	/// Inscribes source rectangle into destination rectangle.
	/// If source rectangle is bigger than destination one then
	/// inscribed will be rectangle with maximum size that can
	/// be in destination rectangle and same aspect ratio as source rectangle.
	/// </summary>
	/// <param name="sourRect">Rectangle that need to inscribed.</param>
	/// <param name="destRect">Rectangle in which sourRect will bee inscribed.</param>
	/// <returns>Returns inscribed rectangle.</returns>
	static DirectX::XMFLOAT2 InscribeRect(const DirectX::XMFLOAT2 &sourRect, const DirectX::XMFLOAT2 &destRect){
		DirectX::XMFLOAT2 inscribed = sourRect;
		bool widthLess = sourRect.x < destRect.x;
		bool heightLess = sourRect.y < destRect.y;
		bool hardCase = !widthLess && !heightLess;

		if (hardCase){
			if (sourRect.x > sourRect.y){
				float invAr = sourRect.y / sourRect.x;

				inscribed.x = destRect.x;
				inscribed.y = destRect.x * invAr;

				if (inscribed.y > destRect.y){
					float hscale = destRect.y / inscribed.y;

					inscribed.x *= hscale;
					inscribed.y *= hscale;
				}
			}
			else{
				float ar = sourRect.x / sourRect.y;

				inscribed.x = destRect.y * ar;
				inscribed.y = destRect.y;

				if (inscribed.x > destRect.x){
					float wscale = destRect.x / inscribed.x;

					inscribed.x *= wscale;
					inscribed.y *= wscale;
				}
			}
		}
		else{
			if (widthLess && !heightLess){
				float hscale = destRect.y / sourRect.y;

				inscribed.x *= hscale;
				inscribed.y *= hscale;
			}

			if (!widthLess && heightLess){
				float wscale = destRect.x / sourRect.x;

				inscribed.x *= wscale;
				inscribed.y *= wscale;
			}
		}

		return inscribed;
	}

	/*
	DirectXMath have bug in BoundingOrientedBox::Transform method:
	http://xboxforums.create.msdn.com/forums/p/113061/680807.aspx
	This implementation have fix for bug.
	*/
	static void BoundingOrientedBoxTransform(const DirectX::BoundingOrientedBox &_this, DirectX::BoundingOrientedBox &Out, DirectX::CXMMATRIX M){
		// Load the box.
		DirectX::XMVECTOR vCenter = DirectX::XMLoadFloat3(&_this.Center);
		DirectX::XMVECTOR vExtents = DirectX::XMLoadFloat3(&_this.Extents);
		DirectX::XMVECTOR vOrientation = DirectX::XMLoadFloat4(&_this.Orientation);

		assert(DirectX::Internal::XMQuaternionIsUnit(vOrientation));

		// Composite the box rotation and the transform rotation.
		DirectX::XMMATRIX nM;
		nM.r[0] = DirectX::XMVector3Normalize(M.r[0]);
		nM.r[1] = DirectX::XMVector3Normalize(M.r[1]);
		nM.r[2] = DirectX::XMVector3Normalize(M.r[2]);
		nM.r[3] = DirectX::g_XMIdentityR3;
		DirectX::XMVECTOR Rotation = DirectX::XMQuaternionRotationMatrix(nM);
		vOrientation = DirectX::XMQuaternionMultiply(vOrientation, Rotation);

		// Transform the center.
		vCenter = DirectX::XMVector3Transform(vCenter, M);

		// Scale the box extents.
		DirectX::XMVECTOR dX = DirectX::XMVector3Length(M.r[0]);
		DirectX::XMVECTOR dY = DirectX::XMVector3Length(M.r[1]);
		DirectX::XMVECTOR dZ = DirectX::XMVector3Length(M.r[2]);

		// bug:
		/*XMVECTOR VectorScale = XMVectorSelect(dX, dY, g_XMSelect1000);
		VectorScale = XMVectorSelect(VectorScale, dZ, g_XMSelect1100);*/
		// fix:
		DirectX::XMVECTOR VectorScale = DirectX::XMVectorSelect(dY, dX, DirectX::g_XMSelect1000);
		VectorScale = DirectX::XMVectorSelect(dZ, VectorScale, DirectX::g_XMSelect1100);
		vExtents = DirectX::XMVectorMultiply(vExtents, VectorScale);
		// Store the box.
		DirectX::XMStoreFloat3(&Out.Center, vCenter);
		DirectX::XMStoreFloat3(&Out.Extents, vExtents);
		DirectX::XMStoreFloat4(&Out.Orientation, vOrientation);
	}

	static DirectX::ContainmentType BoundingBoxContains(const DirectX::BoundingBox &_this, const DirectX::BoundingOrientedBox& box){
		if (!box.Intersects(_this))
			return DirectX::ContainmentType::DISJOINT;

		DirectX::XMVECTOR vCenter = DirectX::XMLoadFloat3(&_this.Center);
		DirectX::XMVECTOR vExtents = DirectX::XMLoadFloat3(&_this.Extents);

		// Subtract off the AABB center to remove a subtract below
		DirectX::XMVECTOR oCenter = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&box.Center), vCenter);

		DirectX::XMVECTOR oExtents = DirectX::XMLoadFloat3(&box.Extents);
		DirectX::XMVECTOR oOrientation = DirectX::XMLoadFloat4(&box.Orientation);

		assert(DirectX::Internal::XMQuaternionIsUnit(oOrientation));

		DirectX::XMVECTOR Inside = DirectX::XMVectorTrueInt();

		for (size_t i = 0; i < DirectX::BoundingOrientedBox::CORNER_COUNT; ++i)
		{
			DirectX::XMVECTOR C = DirectX::XMVectorAdd(DirectX::XMVector3Rotate(DirectX::XMVectorMultiply(oExtents, DirectX::g_BoxOffset[i]), oOrientation), oCenter);
			DirectX::XMVECTOR d = DirectX::XMVectorAbs(C);
			Inside = DirectX::XMVectorAndInt(Inside, DirectX::XMVectorLessOrEqual(d, vExtents));
		}

		return (DirectX::XMVector3EqualInt(Inside, DirectX::XMVectorTrueInt())) ? DirectX::ContainmentType::CONTAINS : DirectX::ContainmentType::INTERSECTS;
	}

	static DirectX::XMMATRIX XMMatrixFrom(const D2D1_MATRIX_3X2_F &v){
		DirectX::XMMATRIX m = DirectX::XMMatrixIdentity();

		m.r[0].XF = v._11;
		m.r[0].YF = v._12;

		m.r[1].XF = v._21;
		m.r[1].YF = v._22;

		m.r[3].XF = v._31;
		m.r[3].YF = v._32;

		return m;
	}

	static DirectX::XMVECTOR GetAABBSizeFrom(DirectX::CXMMATRIX matrix){
		DirectX::XMVECTOR axisX = DirectX::XMVectorAbs(matrix.r[0]);
		DirectX::XMVECTOR axisY = DirectX::XMVectorAbs(matrix.r[1]);
		DirectX::XMVECTOR axisZ = DirectX::XMVectorAbs(matrix.r[2]);

		DirectX::XMVECTOR addXY = DirectX::XMVectorAdd(axisX, axisY);
		DirectX::XMVECTOR addXYZ = DirectX::XMVectorAdd(addXY, axisZ);

		return addXYZ;
	}

	static int RoundUp(int numToRound, int multiple)
	{
		if (multiple == 0)
		{
			return numToRound;
		}

		int remainder = numToRound % multiple;
		if (remainder == 0)
		{
			return numToRound;
		}

		return numToRound + multiple - remainder;
	}

	static float GetRadsToDegsScale() {
		const float scale = (180.0f / DirectX::XM_PI);
		return scale;
	}

	static float GetDegsToRadsScale() {
		const float scale = (DirectX::XM_PI / 180.0f);
		return scale;
	}

	static DirectX::XMMATRIX XMMatrixPerspectiveLH(
		float ViewWidth,
		float ViewHeight,
		float ViewZ,
		float NearZ,
		float FarZ) 
	{
		float _viewZ = ViewZ * NearZ;
		return DirectX::XMMatrixPerspectiveLH(ViewWidth * _viewZ, ViewHeight * _viewZ, NearZ, FarZ);
	}

	// round up for power of two multiples
	template<class T> static T RoundUpPOT(T numToRound, T potMultiple) {
		T res = (numToRound + potMultiple - 1) & ~(potMultiple - 1);
		return res;
	}

	template<class T> static T RoundUpPOT(T v){
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v++;
		return v;
	}

	// Using round up +0.5
	template<class T> static T Convert(T value, T valueUnits, T dstUnits) {
		/*T tmp = value * dstUnits;
		T res = (tmp / valueUnits) + !!(tmp % valueUnits);*/

		/*T tmp = value * dstUnits * byteSize;
		T res = (tmp + (valueUnits / 2)) / (valueUnits * byteSize);*/

		T tmp = value * dstUnits;
		T tmp2 = valueUnits / 2;
		T res = (tmp + tmp2) / valueUnits;


		/*double ratio = (double) dstUnits / (double) valueUnits;
		T res = (T)((double) value * ratio);*/

		/*T tmp = value * dstUnits;
		T res = (tmp) / valueUnits;*/

		return res;
	}

	// Drop fractional part
	template<class T> static T Convert2(T value, T valueUnits, T dstUnits) {
		/*T tmp = value * dstUnits;
		T res = (tmp / valueUnits) + !!(tmp % valueUnits);*/

		/*T tmp = value * dstUnits * byteSize;
		T res = (tmp + (valueUnits / 2)) / (valueUnits * byteSize);*/

		T tmp = value * dstUnits;
		T res = (tmp) / valueUnits;


		/*double ratio = (double) dstUnits / (double) valueUnits;
		T res = (T)((double) value * ratio);*/

		/*T tmp = value * dstUnits;
		T res = (tmp) / valueUnits;*/

		return res;
	}

	template<class T>
	static T Clamp(T value, T min, T max) {
		T res = (std::min)((std::max)(min, value), max);
		return res;
	}

	template<class T>
	static T Lerp(T start, T end, T t) {
		T res = start + (end - start) * t;
		return res;
	}
};