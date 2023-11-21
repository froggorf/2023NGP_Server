#include <Windows.h>
#include "ProcessClientInput.h"
#include "ClientKeyInput.h"
#include "Global.h"


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


void ProcessClientInput()
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
		
		if (dwDirection) {
			PlayerMove(i, dwDirection,)
		}


	}
}
