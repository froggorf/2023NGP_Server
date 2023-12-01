#pragma once
#include "ProcessClientInput.h"
#include "Object.h"
#include "Global.h"


#define DIRECT_FORWARD 0x01
#define DIRECT_BACKWARD 0x02
#define DIRECT_LEFT 0x04
#define DIRECT_RIGHT 0x08
#define DIRECT_UP 0x010
#define DIRECT_DOWN 0x20

#define PLAYER_COLLISION_LENGTH 20.0f
#define PLAYER_COLLISION_OFFSET 0.001f

#define PLAYER_HEIGHT CUBE_WIDTH * 8 / 5
#define PLAYER_WIDTH CUBE_WIDTH * 2 / 3

class CPlayer : public CObject {
protected:
	CObject** m_ppObjects = NULL;
	int m_nObjects = 0;

	DirectX::XMFLOAT3 m_xmf3_Position;
	DirectX::XMFLOAT3 m_xmf3_Right;
	DirectX::XMFLOAT3 m_xmf3_Up;
	DirectX::XMFLOAT3 m_xmf3_Look;

	float m_fPitch;
	float m_fYaw;
	float m_fRoll;

	DirectX::XMFLOAT3 m_xmf3_Velocity;
	DirectX::XMFLOAT3 m_xmf3_Gravity;

	float m_fMax_Velocity;
	float m_fMax_Gravity;

	float m_fFriction;

	LPVOID m_pPlayer_Udt_Context;
	LPVOID m_pCamera_Udt_Context;

	//
	DirectX::XMFLOAT3 m_xmf3_Calculated_Vel;

	//
	bool m_bAble_2_Jump = false;

public:
	CPlayer();
	virtual ~CPlayer();

	DirectX::XMFLOAT3 Get_Position() { return m_xmf3_Position; }
	DirectX::XMFLOAT3 Get_Look_Vector() { return m_xmf3_Look; }
	DirectX::XMFLOAT3 Get_Up_Vector() { return m_xmf3_Up; }
	DirectX::XMFLOAT3 Get_Right_Vector() { return m_xmf3_Right; }


	void Set_Look_Vector(DirectX::XMFLOAT3& xmf3_Look) { m_xmf3_Look = xmf3_Look; }
	void Set_Right_Vector(DirectX::XMFLOAT3& xmf3_Look) { m_xmf3_Right = Vector3::Cross_Product(m_xmf3_Up, xmf3_Look, true); }

	void Set_Velocity(DirectX::XMFLOAT3& xmf3_Velocity) { m_xmf3_Velocity = xmf3_Velocity; }	
	void Set_Gravity(DirectX::XMFLOAT3& xmf3_Gravity) { m_xmf3_Gravity = xmf3_Gravity; }
	void Set_Max_Velocity(float fMax_Velocity) { m_fMax_Velocity = fMax_Velocity; }
	void Set_Max_Gravity(float fMax_Gravity) { m_fMax_Gravity = fMax_Gravity; }
	void Set_Friction(float fFriction) { m_fFriction = fFriction; }

	void Set_Position(DirectX::XMFLOAT3& xmf3_Position) { Move(DirectX::XMFLOAT3(xmf3_Position.x - m_xmf3_Position.x, xmf3_Position.y - m_xmf3_Position.y, xmf3_Position.z - m_xmf3_Position.z), false); }

	DirectX::XMFLOAT3& Get_Velocity() { return m_xmf3_Velocity; }
	float Get_Yaw() { return m_fYaw; }
	float Get_Pitch() { return m_fPitch; }
	float Get_Roll() { return m_fRoll; }


	void Move(int PlayerNumber, float fDistance, bool bVelocity = false);
	void Move(DirectX::XMFLOAT3& xmf3_Shift, bool bVelocity = false);
	void Move(float fOffset_x = 0.0f, float fOffset_y = 0.0f, float fOffset_z = 0.0f);

	void Rotate(float x, float y, float z);

	void Update(int PlayerNumber, float fElapsed_Time);

	virtual void Player_Udt_Callback(float fElapsed_Time) {}
	void Set_Player_Udt_Context(LPVOID pContext) { m_pPlayer_Udt_Context = pContext; }

	//
	//virtual void Prcs_Collision(CObject* pObject);
	virtual void Udt_N_Prcs_Collision(CObject** ppObject, int nObjects);
	void Prepare_Render();
};


//
class CCube_Player : public CPlayer {
public:
	CCube_Player()
	{
		CMesh* pCube_Mesh = new CCube_Mesh(PLAYER_WIDTH, PLAYER_HEIGHT, PLAYER_WIDTH);
		Set_Position(DirectX::XMFLOAT3(0.0f, 50.0f, 0.0f));
	}
	~CCube_Player() {}
};