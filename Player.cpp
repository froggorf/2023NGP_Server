#define _CRT_SECURE_NO_WARNINGS

#include "Player.h"
#include "ClientKeyInput.h"


struct Player_Info {
	float fPosition_x, fPosition_y, fPosition_z;
	float fLook_x, fLook_z;
};
struct Player_Info Player_Info[MAXPLAYERCOUNT];

//===========================================
CPlayer::CPlayer() {

	m_xmf3_Position = DirectX::XMFLOAT3(0.0f, 50.0f, 0.0f);
	m_xmf3_Right = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3_Up = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3_Look = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_xmf3_Velocity = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3_Gravity = DirectX::XMFLOAT3(0.0f, -PLAYER_GRAVITY, 0.0f);
	m_fMax_Velocity = PLAYER_MAX_VELOCITY;
	m_fMax_Gravity = PLAYER_MAX_GRAVITY;
	m_fFriction = PLAYER_FRICTION;

	m_xmf3_Calculated_Vel = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

	m_fPitch = 0.0f;
	m_fYaw = 0.0f;
	m_fRoll = 0.0f;

	m_pPlayer_Udt_Context = NULL;
	m_pCamera_Udt_Context = NULL;
}

CPlayer::~CPlayer() {

}

void CPlayer::Move(int PlayerNumber, float fDistance, bool bVelocity) {
	DirectX::XMFLOAT3 xmf3_Shift = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

	if (GetKeyBuffer(PlayerNumber, CUSTOM_VK_W)) {
		xmf3_Shift = Vector3::Add(xmf3_Shift, m_xmf3_Look, fDistance);
	}
	if (GetKeyBuffer(PlayerNumber, CUSTOM_VK_S)) {
		xmf3_Shift = Vector3::Add(xmf3_Shift, m_xmf3_Look, -fDistance);
	}
	if (GetKeyBuffer(PlayerNumber, CUSTOM_VK_D)) {
		xmf3_Shift = Vector3::Add(xmf3_Shift, m_xmf3_Right, fDistance);
	}
	if (GetKeyBuffer(PlayerNumber, CUSTOM_VK_A)) {
		xmf3_Shift = Vector3::Add(xmf3_Shift, m_xmf3_Right, -fDistance);
	}

	if (m_bAble_2_Jump) {
		if (GetKeyBuffer(PlayerNumber, CUSTOM_VK_SPACE)) {
			m_bAble_2_Jump = false;
			SetKeyBuffer(PlayerNumber, CUSTOM_VK_SPACE, false);
			m_xmf3_Velocity.y = 0.0f;
			xmf3_Shift = Vector3::Add(xmf3_Shift, m_xmf3_Up, fDistance * CUBE_WIDTH * 1.5f);
		}
	}

	Move(xmf3_Shift, bVelocity);

}

void CPlayer::Move(DirectX::XMFLOAT3& xmf3_Shift, bool bVelocity) {
	if (bVelocity) {
		m_xmf3_Velocity = Vector3::Add(m_xmf3_Velocity, xmf3_Shift);
	}
	else {
		m_xmf3_Position = Vector3::Add(m_xmf3_Position, xmf3_Shift);
	}
}


void CPlayer::Move(float fOffset_x, float fOffset_y, float fOffset_z) {
	DirectX::XMFLOAT3 xmf3_Offset = DirectX::XMFLOAT3(fOffset_x, fOffset_y, fOffset_z);
	Move(xmf3_Offset, false);
}


void CPlayer::Update(int PlayerNumber, float fElapsed_Time) {
	m_xmf3_Velocity = Vector3::Add(m_xmf3_Velocity, Vector3::Multiply(m_xmf3_Gravity, fElapsed_Time, false));
	float fLength = sqrtf(m_xmf3_Velocity.x * m_xmf3_Velocity.x + m_xmf3_Velocity.z * m_xmf3_Velocity.z);
	//float fMax_Velocity = m_fMax_Velocity * fElapsed_Time;

	if (fLength > m_fMax_Velocity) {
		m_xmf3_Velocity.x *= (m_fMax_Velocity / fLength);
		m_xmf3_Velocity.z *= (m_fMax_Velocity / fLength);
	}

	//float fMax_Gravity = m_fMax_Gravity * fElapsed_Time;
	fLength = sqrtf(m_xmf3_Velocity.y * m_xmf3_Velocity.y);

	if (fLength > m_fMax_Gravity) {
		m_xmf3_Velocity.y *= (m_fMax_Gravity / fLength);
	}

	m_xmf3_Calculated_Vel = Vector3::Multiply(m_xmf3_Velocity, fElapsed_Time, false);
	//Move(m_xmf3_Calculated_Vel, false);

	if (m_pPlayer_Udt_Context) {
		Player_Udt_Callback(fElapsed_Time);
	}


	fLength = Vector3::Length(m_xmf3_Velocity);
	float fDeceleration = m_fFriction * fElapsed_Time;

	if (fDeceleration > fLength) {
		fDeceleration = fLength;
	}

	//
	DirectX::XMFLOAT3 xmf3_Friction = Vector3::Add(m_xmf3_Velocity, Vector3::Multiply(m_xmf3_Velocity, -fDeceleration, true));

	// 바닥에 닿아 있을시만 점프 가능하게

	if (m_bAble_2_Jump) {
		m_xmf3_Velocity.x = xmf3_Friction.x;
		m_xmf3_Velocity.z = xmf3_Friction.z;
	}

	Prepare_Render();
}

void CPlayer::Udt_N_Prcs_Collision(CObject** ppObject, int nObjects, int PlayerNumber) {
	DirectX::XMFLOAT3 xmf3_Player_Position = Get_Position();
	DirectX::XMFLOAT3 xmf3_Cube_Position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	float fLength = 0.0f;

	std::vector<int> vCrashed_Objs;

	for (int i = 0; i < nObjects; ++i) {
		xmf3_Cube_Position = ppObject[i]->Get_Position();

		fLength = Vector3::Length(Vector3::Subtract(xmf3_Player_Position, xmf3_Cube_Position));

		if (fLength < PLAYER_COLLISION_LENGTH) {
			vCrashed_Objs.push_back(i);
		}
	}

	DirectX::BoundingOrientedBox d3d_OBB_Player;
	DirectX::BoundingOrientedBox d3d_OBB_Object;

	// axis Y
	Move(0.0f, m_xmf3_Calculated_Vel.y, 0.0f);
	Prepare_Render();

	//
	for (int& num : vCrashed_Objs) {
		d3d_OBB_Player = Get_OBB(0);
		d3d_OBB_Object = ppObject[num]->Get_OBB(1);

		if (d3d_OBB_Player.Intersects(d3d_OBB_Object)) {
			DirectX::XMFLOAT3 xmf3_Object_Position = ppObject[num]->Get_Position();
			float fNew_Position_Y = 0.0f;

			if (m_xmf3_Calculated_Vel.y < 0) {
				m_xmf4x4_World._42 = m_xmf3_Position.y = xmf3_Object_Position.y + CUBE_WIDTH / 2 + PLAYER_HEIGHT / 2 + PLAYER_COLLISION_OFFSET;

				m_bAble_2_Jump = true;
			}
			else {
				m_xmf4x4_World._42 = m_xmf3_Position.y = xmf3_Object_Position.y - CUBE_WIDTH / 2 - PLAYER_HEIGHT / 2 - PLAYER_COLLISION_OFFSET;
			}
		}
	}

	// **axis x**
	if (m_xmf3_Position.x < CUBE_WIDTH * 10 && m_xmf3_Position.x > -(CUBE_WIDTH * 10)) {
		// axis x
		Move(m_xmf3_Calculated_Vel.x, 0.0f, 0.0f);
	}
	else
	{
		if (m_xmf3_Position.x > CUBE_WIDTH * 10) {
			// axis x
			Move(-0.05f, 0.0f, 0.0f);
		}
		else {
			// axis x
			Move(0.05f, 0.0f, 0.0f);
		}
	}
	Prepare_Render();

	//
	for (int& num : vCrashed_Objs) {
		d3d_OBB_Player = Get_OBB(0);
		d3d_OBB_Object = ppObject[num]->Get_OBB(1);

		if (d3d_OBB_Player.Intersects(d3d_OBB_Object)) {
			DirectX::XMFLOAT3 xmf3_Object_Position = ppObject[num]->Get_Position();
			float fNew_Position_X = 0.0f;

			if (m_xmf3_Calculated_Vel.x < 0) {
				m_xmf4x4_World._41 = m_xmf3_Position.x = xmf3_Object_Position.x + CUBE_WIDTH / 2 + PLAYER_WIDTH / 2 + PLAYER_COLLISION_OFFSET;
			}
			else {
				m_xmf4x4_World._41 = m_xmf3_Position.x = xmf3_Object_Position.x - CUBE_WIDTH / 2 - PLAYER_WIDTH / 2 - PLAYER_COLLISION_OFFSET;
			}
		}
	}



	// **axis z**
	if (m_xmf3_Position.z < CUBE_WIDTH * 10 && m_xmf3_Position.z > -(CUBE_WIDTH * 10)) {
		// axis z
		Move(0.0f, 0.0f, m_xmf3_Calculated_Vel.z);
	}
	else
	{
		if (m_xmf3_Position.z > CUBE_WIDTH * 10) {
			// axis x
			Move(0.0f, 0.0f, -0.05f);
		}
		else {
			// axis x
			Move(0.0f, 0.0f, 0.05f);
		}
	}
	Prepare_Render();
	//
	for (int& num : vCrashed_Objs) {
		d3d_OBB_Player = Get_OBB(0);
		d3d_OBB_Object = ppObject[num]->Get_OBB(1);

		if (d3d_OBB_Player.Intersects(d3d_OBB_Object)) {
			DirectX::XMFLOAT3 xmf3_Object_Position = ppObject[num]->Get_Position();
			float fNew_Position_Z = 0.0f;

			if (m_xmf3_Calculated_Vel.z < 0) {
				m_xmf4x4_World._43 = m_xmf3_Position.z = xmf3_Object_Position.z + CUBE_WIDTH / 2 + PLAYER_WIDTH / 2 + PLAYER_COLLISION_OFFSET;
			}
			else {
				m_xmf4x4_World._43 = m_xmf3_Position.z = xmf3_Object_Position.z - CUBE_WIDTH / 2 - PLAYER_WIDTH / 2 - PLAYER_COLLISION_OFFSET;
			}
		}
	}

	//

}


void CPlayer::Prepare_Render() {
	m_xmf4x4_World._11 = m_xmf3_Right.x;
	m_xmf4x4_World._12 = m_xmf3_Right.y;
	m_xmf4x4_World._13 = m_xmf3_Right.z;

	m_xmf4x4_World._21 = m_xmf3_Up.x;
	m_xmf4x4_World._22 = m_xmf3_Up.y;
	m_xmf4x4_World._23 = m_xmf3_Up.z;

	m_xmf4x4_World._31 = m_xmf3_Look.x;
	m_xmf4x4_World._32 = m_xmf3_Look.y;
	m_xmf4x4_World._33 = m_xmf3_Look.z;

	m_xmf4x4_World._41 = m_xmf3_Position.x;
	m_xmf4x4_World._42 = m_xmf3_Position.y;
	m_xmf4x4_World._43 = m_xmf3_Position.z;
}



bool CPlayer::Check_Collision_Add_Cube(CObject* pObject, int PlayerNumber) {
	DirectX::XMFLOAT3 xmf3_Player_Position = Get_Position();
	DirectX::XMFLOAT3 xmf3_Cube_Position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	

	DirectX::BoundingOrientedBox d3d_OBB_Player;
	DirectX::BoundingOrientedBox d3d_OBB_Object;

	//

	d3d_OBB_Player = Get_OBB(0);
	d3d_OBB_Object = pObject->Get_OBB(1);
	if (d3d_OBB_Player.Intersects(d3d_OBB_Object)) 
	{
		return true;
	}
	return false;
	
}