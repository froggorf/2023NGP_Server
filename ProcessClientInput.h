#pragma once
#include <DirectXMath.h>

#define PLAYER_MOVE_DISTANCE 100.0f

void ProcessClientInput(float ElapsedTime);
void PlayerMove(int PlayerNumber, ULONG dwDirection, float fDistance,bool bVelocity);

// about calculate
namespace Vector3 {

	inline DirectX::XMFLOAT3 XMVector_2_Float3(DirectX::XMVECTOR& xmv_Vector) {	// vector -> float3
		DirectX::XMFLOAT3 xmf3_Result;
		DirectX::XMStoreFloat3(&xmf3_Result, xmv_Vector);

		return xmf3_Result;
	}

	inline DirectX::XMFLOAT3 Multiply(DirectX::XMFLOAT3& xmf3_Vector, float fScalar, bool bNormalize = true) {	// float3 * float
		DirectX::XMFLOAT3 xmf3_Result;

		if (bNormalize) {
			DirectX::XMStoreFloat3(&xmf3_Result, DirectX::XMVectorScale(DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&xmf3_Vector)), fScalar));
		}
		else {
			DirectX::XMStoreFloat3(&xmf3_Result, DirectX::XMVectorScale(DirectX::XMLoadFloat3(&xmf3_Vector), fScalar));
		}

		return xmf3_Result;
	}

	inline DirectX::XMFLOAT3 Add(const DirectX::XMFLOAT3& xmf3_Vector1, const DirectX::XMFLOAT3& xmf3_Vector2) {	// float3 + float3
		DirectX::XMFLOAT3 xmf3_Result;
		DirectX::XMStoreFloat3(&xmf3_Result, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&xmf3_Vector1), DirectX::XMLoadFloat3(&xmf3_Vector2)));

		return xmf3_Result;
	}

	inline DirectX::XMFLOAT3 Add(DirectX::XMFLOAT3& xmf3_Vector1, DirectX::XMFLOAT3& xmf3_Vector2, float fScalar) {	// float3 + float3 * float
		DirectX::XMFLOAT3 xmf3_Result;
		DirectX::XMStoreFloat3(&xmf3_Result, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&xmf3_Vector1), DirectX::XMVectorScale(DirectX::XMLoadFloat3(&xmf3_Vector2), fScalar)));

		return xmf3_Result;
	}

	inline DirectX::XMFLOAT3 Subtract(DirectX::XMFLOAT3& xmf3_Vector1, DirectX::XMFLOAT3& xmf3_Vector2) {	// float3 - float3
		DirectX::XMFLOAT3 xmf3_Result;
		DirectX::XMStoreFloat3(&xmf3_Result, DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&xmf3_Vector1), DirectX::XMLoadFloat3(&xmf3_Vector2)));

		return xmf3_Result;
	}

	inline float Dot_Product(DirectX::XMFLOAT3& xmf3_Vector1, DirectX::XMFLOAT3& xmf3_Vector2) {	// float3 dot float3
		DirectX::XMFLOAT3 xmf3_Result;
		DirectX::XMStoreFloat3(&xmf3_Result, DirectX::XMVector3Dot(DirectX::XMLoadFloat3(&xmf3_Vector1), DirectX::XMLoadFloat3(&xmf3_Vector2)));

		return xmf3_Result.x;
	}

	inline DirectX::XMFLOAT3 Cross_Product(DirectX::XMFLOAT3& xmf3_Vector1, DirectX::XMFLOAT3& xmf3_Vector2, bool bNormalize = true) {	// float3 cross float3
		DirectX::XMFLOAT3 xmf3_Result;

		if (bNormalize) {
			DirectX::XMStoreFloat3(&xmf3_Result, DirectX::XMVector3Normalize(DirectX::XMVector3Cross(DirectX::XMLoadFloat3(&xmf3_Vector1), DirectX::XMLoadFloat3(&xmf3_Vector2))));
		}
		else {
			DirectX::XMStoreFloat3(&xmf3_Result, DirectX::XMVector3Cross(DirectX::XMLoadFloat3(&xmf3_Vector1), DirectX::XMLoadFloat3(&xmf3_Vector2)));
		}

		return xmf3_Result;
	}

	inline DirectX::XMFLOAT3 Normalize(DirectX::XMFLOAT3& xmf3_Vector) {	// normalize float3
		DirectX::XMFLOAT3 xmf3_Result;
		DirectX::XMStoreFloat3(&xmf3_Result, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&xmf3_Vector)));

		return xmf3_Result;
	}

	inline float Length(DirectX::XMFLOAT3& xmf3_Vector) {	// length of float3
		DirectX::XMFLOAT3 xmf3_Result;
		DirectX::XMStoreFloat3(&xmf3_Result, DirectX::XMVector3Length(DirectX::XMLoadFloat3(&xmf3_Vector)));

		return xmf3_Result.x;
	}

	inline float Angle(DirectX::XMVECTOR& xmv_Vector1, DirectX::XMVECTOR& xmv_Vector2) {	// angle btw vector & vector
		DirectX::XMVECTOR xmvAngle = DirectX::XMVector3AngleBetweenNormals(xmv_Vector1, xmv_Vector2);
		return DirectX::XMConvertToDegrees(acosf(DirectX::XMVectorGetX(xmvAngle)));
	}

	

	inline DirectX::XMFLOAT3 Transform_Normal(DirectX::XMFLOAT3& xmf3_Vector, DirectX::XMMATRIX& xmmtx_Transform) {
		DirectX::XMFLOAT3 xmf3_Result;
		DirectX::XMStoreFloat3(&xmf3_Result, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&xmf3_Vector), xmmtx_Transform));

		return xmf3_Result;
	}

	inline DirectX::XMFLOAT3 Transform_Coord(DirectX::XMFLOAT3& xmf3_Vector, DirectX::XMMATRIX& xmmtx_Transform) {
		DirectX::XMFLOAT3 xmf3_Result;
		DirectX::XMStoreFloat3(&xmf3_Result, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&xmf3_Vector), xmmtx_Transform));

		return xmf3_Result;
	}

	
}