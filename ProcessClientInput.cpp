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


#define VK_W 0x57
#define VK_A 0x41
#define VK_S 0x53
#define VK_D 0x44

struct Player_Info {
	float fPosition_x, fPosition_y, fPosition_z;
	float fLook_x, fLook_z;
};
struct Player_Info Player_Info[MAXPLAYERCOUNT];

void ProcessClientInput(float ElapsedTime)
{
	for (int i = 0; i < MAXPLAYERCOUNT; ++i) {
		DWORD dwDirection = 0;
	
		if (clientKeyBuffer[i][VK_W] & 0xF0) {
			dwDirection |= DIRECT_FORWARD;
		}
		if (clientKeyBuffer[i][VK_S] & 0xF0) {
			dwDirection |= DIRECT_BACKWARD;
		}
		if (clientKeyBuffer[i][VK_A] & 0xF0) {
			dwDirection |= DIRECT_LEFT;
		}
		if (clientKeyBuffer[i][VK_D] & 0xF0) {
			dwDirection |= DIRECT_RIGHT;
		}
		if (clientKeyBuffer[i][VK_SPACE] & 0xF0) {
			dwDirection |= DIRECT_UP;
		}
		if (clientKeyBuffer[i][VK_LSHIFT] & 0xF0) {
			dwDirection |= DIRECT_DOWN;
		}

		static int a = 3;
		a += 1;
		if (i == 0 && a%10==0) {
			printf("%d 번째 플레이어 %d의 움직임\n", i, dwDirection);
		}
		
		if (dwDirection != 0) {
			PlayerMove(i, dwDirection, PLAYER_MOVE_DISTANCE * ElapsedTime, false);
		}


	}
}

void PlayerMove(int PlayerNumber, ULONG dwDirection, float fDistance, bool bVelocity) {
	if (dwDirection) {
		DirectX::XMFLOAT3 xmf3_Shift = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

		// look벡터 , up벡터, right벡터 구하기
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