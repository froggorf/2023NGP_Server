#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <time.h>
#include <chrono>



// 구조체
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
// 변수
//int Cube_num = 0;												// 설치된 큐브 개수
int Current_Player_Count = 0;										// 접속한 플레이어 인원 수 
struct sockaddr_in clientAddr[MAXPLAYERCOUNT];					// 클라이언트 주소 정보

//std::vector<Player_Info*> Player_Info;							// 플레이어 정보
extern struct Player_Info Player_Info[MAXPLAYERCOUNT];

std::vector<SOCKET> socket_vector;								// 각 쓰레드에서 얻은 소켓 정보(TCP, time)
std::vector<Cube_Info> Total_Cube;								// 전체 큐브 정보
std::vector<SOCKET> socket_Cube_vector;							// 각 쓰레드에서 얻은 소켓 정보(TCP, Cube)

std::vector<SOCKET> socket_SendPlayerData_vector;
std::vector<SOCKET> socket_RecvLookVector;