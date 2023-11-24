#include <Windows.h>
#include "ProcessClientInput.h"
#include "ClientKeyInput.h"
#include "Global.h"

#include <iostream>

#define DIRECT_FORWARD 0x01
#define DIRECT_BACKWARD 0x02
#define DIRECT_LEFT 0x04
#define DIRECT_RIGHT 0x08
#define DIRECT_UP 0x010
#define DIRECT_DOWN 0x20



struct Player_Info {
	float fPosition_x, fPosition_y, fPosition_z;
	float fLook_x, fLook_z;
};
struct Player_Info Player_Info[MAXPLAYERCOUNT];

void ProcessClientInput(float ElapsedTime)
{
	for (int i = 0; i < MAXPLAYERCOUNT; ++i) {
		DWORD dwDirection = 0;
		if (GetKeyBuffer(i, VK_W)) {
			dwDirection |= DIRECT_FORWARD;
		}
		if (GetKeyBuffer(i, VK_S)) {
			dwDirection |= DIRECT_BACKWARD;
		}
		if (GetKeyBuffer(i, VK_A)) {
			dwDirection |= DIRECT_LEFT;
		}
		if (GetKeyBuffer(i, VK_D)) {
			dwDirection |= DIRECT_RIGHT;
		}
		if (GetKeyBuffer(i, VK_SPACE)) {
			dwDirection |= DIRECT_UP;
		}
		if (GetKeyBuffer(i, VK_LSHIFT)) {
			dwDirection |= DIRECT_DOWN;
		}
		
		
		if (dwDirection != 0) {
			PlayerMove(i, dwDirection, PLAYER_MOVE_DISTANCE * ElapsedTime, false);
		}


	}
}

void PlayerMove(int PlayerNumber, ULONG dwDirection, float fDistance, bool bVelocity) {
	if (dwDirection) {
		DirectX::XMFLOAT3 xmf3_Shift = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

		// lookº¤ÅÍ , upº¤ÅÍ, rightº¤ÅÍ ±¸ÇÏ±â
		DirectX::XMFLOAT3 xmf3_Look = DirectX::XMFLOAT3(Player_Info[PlayerNumber].fLook_x, 0.0f, (Player_Info[PlayerNumber].fLook_z));
		DirectX::XMFLOAT3 xmf3_Up = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
		DirectX::XMFLOAT3 xmf3_Right = DirectX::XMFLOAT3(0.0f,0.0f,0.0f);
		xmf3_Right = Vector3::Cross_Product(xmf3_Up, xmf3_Look, true);

		if (dwDirection & DIRECT_FORWARD) {
			xmf3_Shift = Vector3::Add(xmf3_Shift, xmf3_Look, fDistance);
		}
		if (dwDirection & DIRECT_BACKWARD) {
			xmf3_Shift = Vector3::Add(xmf3_Shift, xmf3_Look, -fDistance);
		}
		if (dwDirection & DIRECT_RIGHT) {
			xmf3_Shift = Vector3::Add(xmf3_Shift, xmf3_Right, fDistance);
		}
		if (dwDirection & DIRECT_LEFT) {
			xmf3_Shift = Vector3::Add(xmf3_Shift, xmf3_Right, -fDistance);
		}

		if (dwDirection & DIRECT_UP) {
			xmf3_Shift = Vector3::Add(xmf3_Shift, xmf3_Up, fDistance);
		}
		if (dwDirection & DIRECT_DOWN) {
			xmf3_Shift = Vector3::Add(xmf3_Shift, xmf3_Up, -fDistance);
		}

		//Move(PlayerNumber, xmf3_Shift, bVelocity);
		Player_Info[PlayerNumber].fPosition_x += xmf3_Shift.x;
		Player_Info[PlayerNumber].fPosition_y += xmf3_Shift.y;
		Player_Info[PlayerNumber].fPosition_z += xmf3_Shift.z;
	}
}

//void Move(int PlayerNumber, DirectX::XMFLOAT3& xmf3_Shift, bool bVelocity) {
//	if (bVelocity) {
//		m_xmf3_Velocity = Vector3::Add(m_xmf3_Velocity, xmf3_Shift);
//	}
//	else {
//		
//
//		if (m_pCamera) {
//			m_pCamera->Move(xmf3_Shift);
//		}
//	}
//}