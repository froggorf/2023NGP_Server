#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <time.h>
#include <chrono>
#include "Global.h"
#include "Player.h"


// ========================== ����ü �� Ŭ���� ==========================
extern struct Player_Info{
	float fPosition_x, fPosition_y, fPosition_z;
	float fLook_x, fLook_z;
};

struct Cube_Info {
	float fPosition_x, fPosition_y, fPosition_z;
	float fColor_r, fColor_g, fColor_b;
	bool AddorDelete;
};

struct Look_Data
{
	int PlayerNumber;
	float fLook_x, fLook_z;
};

class ChatString
{
public:
	ChatString() {}
	~ChatString() {}
	ChatString(int pn, std::string cd)
	{
		playerNumber = pn;
		for(int i=0; i<cd.size(); ++i)
		{
			chatData[i] = cd[i];
		}
		chatData[cd.size()] = '\0';
	};
public:
	int playerNumber{};       // -1 -> Ŀ�ǵ� ä��, 0 -> 0�� �÷��̾�, 1 -> 1�� �÷��̾�,
	// 2  -> 2�� �÷��̾�, ..., MAX_PLAYER_COUNT-> �������ν� ä��
	char chatData[CHATMAXBUFFER+1]={0,};
};

// ========================== �Լ� ==========================



// ========================== ���� ==========================
CPlayer* vPlayer = new CPlayer[MAXPLAYERCOUNT];

//CCube_Player* pCube_Player = new CCube_Player();
CObject** ppObjects;
int nObjects;

int Current_Player_Count = 0;									// ���� ������ �÷��̾� �ο� �� 
struct sockaddr_in clientAddr[MAXPLAYERCOUNT];					// Ŭ���̾�Ʈ �ּ� ����					
extern struct Player_Info Player_Info[MAXPLAYERCOUNT];			// �÷��̾� ����(��ġ, �躤��) �迭

std::vector<SOCKET> socket_vector;								// �ð� ���� ���� ����
std::vector<Cube_Info> Total_Cube;								// ��ü ť�� ����
std::vector<SOCKET> socket_Cube_vector;							// ť�� ��ġ ���� ���� �޾� ��� Ŭ�󿡰� �����ϴ� ����
std::vector<SOCKET> socket_SendPlayerData_vector;				// �÷��̾� ����(��ġ, �躤��) ���� ����
std::vector<SOCKET> socket_chat_vector;							// ä�� ���� �޾� Echo �ϴ� ����