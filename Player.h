#pragma once
#include "ProcessClientInput.h"
#include "Object.h"
#include "Global.h"


#define PLAYER_MAX_VELOCITY 75.0f
#define PLAYER_MAX_GRAVITY 75.0f
#define PLAYER_GRAVITY 100.0f
#define PLAYER_FRICTION 500.0f
#define PLAYER_MOVE_DISTANCE 200.0f
#define PLAYER_PICKING_DISTANCE 100.0f

#define PLAYER_COLLISION_LENGTH 20.0f
#define PLAYER_COLLISION_OFFSET 0.001f

#define PLAYER_HEIGHT CUBE_WIDTH * 8 / 5
#define PLAYER_WIDTH CUBE_WIDTH * 2 / 3

class CPlayer : public CObject {
protected:

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
	bool check_able_Jump = false;

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


	void Update(int PlayerNumber, float fElapsed_Time);

	virtual void Player_Udt_Callback(float fElapsed_Time) {}
	void Set_Player_Udt_Context(LPVOID pContext) { m_pPlayer_Udt_Context = pContext; }

	//
	virtual void Udt_N_Prcs_Collision(CObject** ppObject, int nObjects, int PlayerNumber);	// 충돌체크
	void Prepare_Render();																	// 행렬 갱신
	bool Check_Collision_Add_Cube(CObject* pObject, int PlayerNumber);						// add cube 가능한지 바운딩 박스로 확인
};
