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

struct Cube_Info {
	float fPosition_x, fPosition_y, fPosition_z;
	float fColor_r, fColor_g, fColor_b;
};

// ����
//int Cube_num = 0;												// ��ġ�� ť�� ����
int Current_Player_Count{};										// ������ �÷��̾� �ο� �� 
struct sockaddr_in clientAddr[MAXPLAYERCOUNT];					// Ŭ���̾�Ʈ �ּ� ����
std::vector<Player_Info*> Player_Info;							// �÷��̾� ����
std::vector<SOCKET> socket_vector;								// �� �����忡�� ���� ���� ����(TCP, time)
std::vector<Cube_Info> Total_Cube;								// ��ü ť�� ����
std::vector<SOCKET> socket_Cube_vector;							// �� �����忡�� ���� ���� ����(TCP, Cube)