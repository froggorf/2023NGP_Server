#pragma once
#include "ProcessClientInput.h"
#include "Mesh.h"

// Cube data
#define CUBE_WIDTH 10.0f
#define CUBE_INIT_RING_NUMBER 10
#define CUBE_MAX_NUMBER 10000

class CObject {

protected:
	DirectX::XMFLOAT4X4 m_xmf4x4_World;
	//
	DirectX::XMFLOAT4 m_xmf4_Color;

public:
	CObject();
	virtual ~CObject();

	DirectX::XMFLOAT3 Get_Position();

	//
	DirectX::BoundingOrientedBox Get_OBB();

	void Set_Position(float x, float y, float z);
	void Set_Position(DirectX::XMFLOAT3 xmf3_Position);

	//
	virtual DirectX::XMFLOAT4X4 Get_World_Matrix() { return m_xmf4x4_World; };
};