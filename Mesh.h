#pragma once
#include "ProcessClientInput.h"


class CMesh {


protected:

	D3D12_PRIMITIVE_TOPOLOGY m_d3d_Primitive_Topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UINT m_nSlot = 0;
	UINT m_nVertices = 0;
	UINT m_nStride = 0;
	UINT m_nOffset = 0;


	UINT m_nIndices = 0;
	UINT m_nStart_Index = 0;
	UINT m_nBase_Vertex = 0;

	//
	DirectX::BoundingOrientedBox m_xmOOBB;

	UINT* m_pnIndices = NULL;


public:
	CMesh() = default;
	virtual ~CMesh();


public:
	//
	virtual DirectX::BoundingOrientedBox Get_OBB() { return m_xmOOBB; }

};

 
class CCube_Mesh : public CMesh {
public:
	CCube_Mesh(float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f)
	{
		m_nVertices = 8;

		float fx = fWidth * 0.5f;
		float fy = fHeight * 0.5f;
		float fz = fDepth * 0.5f;

		//
		m_nIndices = 36;
		m_pnIndices = new UINT[m_nIndices];

		// top
		m_pnIndices[0] = 3;
		m_pnIndices[1] = 1;
		m_pnIndices[2] = 0;
		m_pnIndices[3] = 2;
		m_pnIndices[4] = 1;
		m_pnIndices[5] = 3;

		// front
		m_pnIndices[6] = 0;
		m_pnIndices[7] = 5;
		m_pnIndices[8] = 4;
		m_pnIndices[9] = 1;
		m_pnIndices[10] = 5;
		m_pnIndices[11] = 0;

		// left
		m_pnIndices[12] = 3;
		m_pnIndices[13] = 4;
		m_pnIndices[14] = 7;
		m_pnIndices[15] = 0;
		m_pnIndices[16] = 4;
		m_pnIndices[17] = 3;

		// right
		m_pnIndices[18] = 1;
		m_pnIndices[19] = 6;
		m_pnIndices[20] = 5;
		m_pnIndices[21] = 2;
		m_pnIndices[22] = 6;
		m_pnIndices[23] = 1;

		// back
		m_pnIndices[24] = 2;
		m_pnIndices[25] = 7;
		m_pnIndices[26] = 6;
		m_pnIndices[27] = 3;
		m_pnIndices[28] = 7;
		m_pnIndices[29] = 2;

		// bottom
		m_pnIndices[30] = 6;
		m_pnIndices[31] = 4;
		m_pnIndices[32] = 5;
		m_pnIndices[33] = 7;
		m_pnIndices[34] = 4;
		m_pnIndices[35] = 6;


		//
		m_xmOOBB = DirectX::BoundingOrientedBox(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(fx, fy, fz), DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	}
	virtual ~CCube_Mesh();

	//
	virtual DirectX::BoundingOrientedBox Get_OBB()
	{
		return m_xmOOBB;
	}
};

