#pragma once
#include <iostream>
#include <vector>
#include <time.h>
#include <chrono>



// ����ü
struct Player_Info{
	float fPosition_x, fPosition_y, fPosition_z;
	float fLook_x, fLook_z;
};

// ����
int Current_Player_Count{};										// ������ �÷��̾� �ο� �� 
struct sockaddr_in clientAddr[MAXPLAYERCOUNT];					// Ŭ���̾�Ʈ �ּ� ����
std::vector<Player_Info*> Player_Info;							// �÷��̾� ����
std::vector<SOCKET> socket_vector;