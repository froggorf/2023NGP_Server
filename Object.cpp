#include "Object.h"

CObject::CObject() 
{
	DirectX::XMStoreFloat4x4(&m_xmf4x4_World, DirectX::XMMatrixIdentity());
}

CObject::~CObject() 
{

}

DirectX::XMFLOAT3 CObject::Get_Position() {
	return DirectX::XMFLOAT3(m_xmf4x4_World._41, m_xmf4x4_World._42, m_xmf4x4_World._43);
}

void CObject::Set_Position(float x, float y, float z) {
	m_xmf4x4_World._41 = x;
	m_xmf4x4_World._42 = y;
	m_xmf4x4_World._43 = z;
}

void CObject::Set_Position(DirectX::XMFLOAT3 xmf3_Position) {
	Set_Position(xmf3_Position.x, xmf3_Position.y, xmf3_Position.z);
}

#define PLAYER_HEIGHT CUBE_WIDTH * 8 / 5
#define PLAYER_WIDTH CUBE_WIDTH * 2 / 3

DirectX::BoundingOrientedBox CObject::Get_OBB(int nType) 
{
	DirectX::BoundingOrientedBox d3d_OBB;
	DirectX::XMFLOAT4X4 xmf4x4_Transform = Matrix4x4::Identity();

	xmf4x4_Transform._41 = m_xmf4x4_World._41;
	xmf4x4_Transform._42 = m_xmf4x4_World._42;
	xmf4x4_Transform._43 = m_xmf4x4_World._43;

	switch (nType) {
	case 0:
		d3d_OBB = DirectX::BoundingOrientedBox(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(PLAYER_WIDTH * 0.5f, PLAYER_HEIGHT * 0.5f, PLAYER_WIDTH * 0.5f), DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		break;
	case 1:
		d3d_OBB = DirectX::BoundingOrientedBox(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(CUBE_WIDTH * 0.5f, CUBE_WIDTH * 0.5f, CUBE_WIDTH * 0.5f), DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		break;
	default:
		break;
	}

	d3d_OBB.Transform(d3d_OBB, DirectX::XMLoadFloat4x4(&xmf4x4_Transform));

	return d3d_OBB;
}