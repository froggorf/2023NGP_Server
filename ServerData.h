#pragma once
#include <iostream>
#include <vector>
#include <time.h>
#include <chrono>

// 구조체
struct Player_Info{
	float fPosition_x, fPosition_y, fPosition_z;
	float fLook_x, fLook_z;
};

// 변수
int Current_Player_Count{};										// 접속한 플레이어 인원 수 
struct sockaddr_in clientAddr[MAXPLAYERCOUNT];					// 클라이언트 주소 정보
std::vector<Player_Info*> Player_Info;							// 플레이어 정보
std::vector<SOCKET> socket_vector;