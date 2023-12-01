#pragma once
#include <DirectXMath.h>
#include <d3d12.h>
#include <DirectXCollision.h>
#include <vector>




//void ProcessClientInput(float ElapsedTime);
//void PlayerMove(int PlayerNumber, ULONG dwDirection, float fDistance,bool bVelocity);

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


namespace Vector4 {
	inline DirectX::XMFLOAT4 Add(DirectX::XMFLOAT4& xmf4_Vector1, DirectX::XMFLOAT4& xmf4_Vector2) {	// float4 + float4
		DirectX::XMFLOAT4 xmf4_Result;
		DirectX::XMStoreFloat4(&xmf4_Result, DirectX::XMVectorAdd(DirectX::XMLoadFloat4(&xmf4_Vector1), DirectX::XMLoadFloat4(&xmf4_Vector2)));

		return xmf4_Result;
	}

	inline DirectX::XMFLOAT4 Multiply(DirectX::XMFLOAT4& xmf4_Vector1, DirectX::XMFLOAT4& xmf4_Vector2) {	// float4 * float4
		DirectX::XMFLOAT4 xmf4_Result;
		DirectX::XMStoreFloat4(&xmf4_Result, DirectX::XMVectorMultiply(DirectX::XMLoadFloat4(&xmf4_Vector1), DirectX::XMLoadFloat4(&xmf4_Vector2)));

		return xmf4_Result;
	}

	inline DirectX::XMFLOAT4 Multiply(DirectX::XMFLOAT4& xmf4_Vector, float fScalar) {	// float4 * float
		DirectX::XMFLOAT4 xmf4_Result;
		DirectX::XMStoreFloat4(&xmf4_Result, DirectX::XMVectorScale(DirectX::XMLoadFloat4(&xmf4_Vector), fScalar));

		return xmf4_Result;
	}
}

namespace Matrix4x4 {
	inline DirectX::XMFLOAT4X4 Identity() {
		DirectX::XMFLOAT4X4 xmf4x4Result;
		DirectX::XMStoreFloat4x4(&xmf4x4Result, DirectX::XMMatrixIdentity());

		return xmf4x4Result;
	}

	inline DirectX::XMFLOAT4X4 Multiply(DirectX::XMFLOAT4X4& xmmtx4x4_Matrix1, DirectX::XMFLOAT4X4& xmmtx4x4_Matrix2) {
		DirectX::XMFLOAT4X4 xmf4x4Result;
		DirectX::XMStoreFloat4x4(&xmf4x4Result, DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&xmmtx4x4_Matrix1), DirectX::XMLoadFloat4x4(&xmmtx4x4_Matrix2)));

		return xmf4x4Result;
	}

	inline DirectX::XMFLOAT4X4 Multiply(DirectX::XMFLOAT4X4& xmmtx4x4_Matrix1, DirectX::XMMATRIX& xmmtx_Matrix2) {
		DirectX::XMFLOAT4X4 xmf4x4Result;
		DirectX::XMStoreFloat4x4(&xmf4x4Result, DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&xmmtx4x4_Matrix1), xmmtx_Matrix2));

		return xmf4x4Result;
	}

	inline DirectX::XMFLOAT4X4 Multiply(DirectX::XMMATRIX& xmmtx_Matrix1, DirectX::XMFLOAT4X4& xmmtx4x4_Matrix2) {
		DirectX::XMFLOAT4X4 xmf4x4Result;
		DirectX::XMStoreFloat4x4(&xmf4x4Result, DirectX::XMMatrixMultiply(xmmtx_Matrix1, DirectX::XMLoadFloat4x4(&xmmtx4x4_Matrix2)));

		return xmf4x4Result;
	}

	inline DirectX::XMFLOAT4X4 Inverse(DirectX::XMFLOAT4X4& xmmtx4x4_Matrix) {	// Inverse
		DirectX::XMFLOAT4X4 xmf4x4Result;
		DirectX::XMStoreFloat4x4(&xmf4x4Result, DirectX::XMMatrixInverse(NULL, DirectX::XMLoadFloat4x4(&xmmtx4x4_Matrix)));

		return xmf4x4Result;
	}

	inline DirectX::XMFLOAT4X4 Transpose(DirectX::XMFLOAT4X4& xmmtx4x4_Matrix) {	// Transpose
		DirectX::XMFLOAT4X4 xmf4x4Result;
		DirectX::XMStoreFloat4x4(&xmf4x4Result, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&xmmtx4x4_Matrix)));

		return xmf4x4Result;
	}

	inline DirectX::XMFLOAT4X4 PerspectiveFov_LH(float FovAngle_Y, float Aspect_Ratio, float Near_Z, float Far_Z) {	// Projection Matrix
		DirectX::XMFLOAT4X4 xmf4x4Result;
		DirectX::XMStoreFloat4x4(&xmf4x4Result, DirectX::XMMatrixPerspectiveFovLH(FovAngle_Y, Aspect_Ratio, Near_Z, Far_Z));

		return xmf4x4Result;
	}

	inline DirectX::XMFLOAT4X4 LookAt_LH(DirectX::XMFLOAT3& xmf3_Eye_Position, DirectX::XMFLOAT3& xmf3_LookAt_Position, DirectX::XMFLOAT3& xmf3_Up_Direction) {	// View Matrix
		DirectX::XMFLOAT4X4 xmf4x4Result;
		DirectX::XMStoreFloat4x4(&xmf4x4Result, DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&xmf3_Eye_Position), DirectX::XMLoadFloat3(&xmf3_LookAt_Position), DirectX::XMLoadFloat3(&xmf3_Up_Direction)));

		return xmf4x4Result;
	}
}
