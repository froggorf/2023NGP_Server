#include "Common.h"
#include "Global.h"
#include "ServerData.h"
#include "ClientKeyInput.h"
#include "ProcessClientInput.h"
#include "array"

// Timer 관련
#include <Mmsystem.h>
#pragma comment(lib, "winmm.lib")

#include <mutex>

// TODO: 마지막날 출력 관련(printf, cout) 모두 제거할 예정

// ========================== 함수 ==========================
void CreateListenSockets(SOCKET&, SOCKET&, SOCKET&, SOCKET&, SOCKET&);			// 리슨 소켓 생성
void InitGame();																// 게임 데이터 초기화 부분, 재시작 시 다시 호출하여 실행할 수 있도록 구현 예정
void LoginPlayer(SOCKET&, SOCKET&, SOCKET&, SOCKET&, SOCKET&);					// listen_sock accept 및 쓰레드 생성 함수
void ConnectAndAddPlayer(SOCKET&);												// 처음 접속시 플레이어에 대한 정보 전송 및 시간 전송 소켓 생성
void CreateClientKeyInputThread(SOCKET& KeyInput_listen_sock);					// 플레이어 마다 키 인풋 정보 수신 받는 소켓 및 쓰레드 생성
DWORD WINAPI Send_Game_Time(LPVOID arg);										// 시간 업데이트 함수
DWORD WINAPI ProcessClientKeyInput(LPVOID arg);									// 플레이어의 키 입력 정보 송신받아 키 버퍼 갱신
void CreateSendPlayerDataThread(SOCKET& senddata_listen_sock);					// 플레이어 정보(위치,룩벡터) 전송 소켓 생성
DWORD WINAPI SendPlayerDataToClient(LPVOID arg);								// 메인 작업 쓰레드, 플레이어 이동 - 충돌체크 - 모든 클라에게 모든 플레이어 위치 전송
void CreateChatThread(SOCKET& chat_listen_sock);								// 채팅 소켓 및 쓰레드 생성
DWORD WINAPI ProcessEchoChat(LPVOID arg);										// 채팅 정보 수신 받아 모든 클라에게 전송하는 함수
void CreateCubeThread(SOCKET& Cube_listen_sock);								// 각 플레이어 접속시 큐브 쓰레드 생성
DWORD WINAPI EchoClientRequestCube(LPVOID arg);									// 받은 큐브 정보 설치 및 삭제 판별 후 모든 클라이언트에게 다시 돌려보냄
bool Check_Add_Cube(Cube_Info cube);											// 플레이어 주변 바운딩 박스로 충돌체크하여 큐브 설치 가능한지 확인하는 함수
bool CompareXMFLOAT3(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b);	// XMFLOAT3 비교 함수
void ClearAllSocket();															// 게임 초기화 시 모든 소켓 정보 초기화
bool PlayerLogout(int playerNumber);											// 플레이어가 게임 진행중 게임을 종료해 서버에서 나갔을 때 함수
void Set_Floor_Cube_Object();													// 바닥 오브젝트 설정
void Release_Floor_Cube_Object();												// 전체 오브젝트 초기화
void Delete_Cube(Cube_Info clientCubeInput);									// 큐브 삭제 요청 send
void Request_Delete_All_Cube();													// 큐브 전체 삭제 요청


// ========================== 변수 ==========================

WSADATA wsa;																	// 윈속
int remainingSeconds = GAMETIME;												// 게임 시간 
HWND timerHWND;																	// 타이머 핸들
DWORD g_startTime;																// ElapsedTime 관련
DWORD g_prevTime;
CRITICAL_SECTION cs_for_logout;													// PlayerLogout(int) 내부 bool bPlayerLogout[] 사용을 위한 cs
CRITICAL_SECTION cs_Cube;														// cs_Cube 사용을 위한 cs
bool bPlayerLogout[MAXPLAYERCOUNT] = { false, };								// 로그아웃 중복 처리 방지 변수

std::mutex mtx;

int main(int argc, char *argv[])
{
	int retval;
	
	while(true)
	{
		// 큐브 데이터 초기화
		Release_Floor_Cube_Object();
		// 바닥 cube object 생성
		Set_Floor_Cube_Object(); 

		// 리슨 소켓 생성
		SOCKET login_listen, keyinput_listen, cube_listen, playerdata_listen, chat_listen;
		CreateListenSockets(login_listen, keyinput_listen, cube_listen, playerdata_listen, chat_listen);
		
		// 게임 데이터 초기화
		InitGame();


		// 플레이어 지정한 수 인원 접속시키기
		LoginPlayer(login_listen, keyinput_listen, cube_listen, playerdata_listen, chat_listen);

		// 타이머 초기화
		std::cout << "타이머 시작 - " << remainingSeconds << std::endl;
		HANDLE Timer_hThread = CreateThread(NULL, 0, Send_Game_Time,
			(LPVOID)0, 0, NULL);
		CloseHandle(Timer_hThread);

		bool bGame = true;
		while (bGame) {
			// 현재 플레이어 인원 체크 및 없을 경우 종료
			if (Current_Player_Count == 0)
			{
				bGame = false;
				break;
			}
		}
		

		// TODO: 현재는 InitGame()으로 종료 초기화 진행, 필요시 EndGame() 만들기.
		InitGame();
		


	}
	
	// 윈속 종료
	WSACleanup();
	return 0;
}


void ConnectAndAddPlayer(SOCKET& listen_sock)
{
	SOCKET client_sock;
	struct sockaddr_in clientaddr;

	// accept()
	int addrlen = sizeof(clientaddr);
	client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
	if (client_sock == INVALID_SOCKET) {
		err_display("accept()");
		return;
	}

	// 시간 통신용 소켓 벡터에 저장
	if(socket_vector.size() > Current_Player_Count)		// 게임 시작 전 로그아웃 한 기록이 남아있는 것
	{
		for(int i=0; i<socket_vector.size(); ++i)
		{
			if (socket_vector[i] == INVALID_SOCKET) {
				// PlayerNumber 전달
				send(client_sock, (char*)&i, sizeof(Current_Player_Count), 0);
				socket_vector[i] = client_sock;
				Player_Info[i].fPosition_x = 0.0f; Player_Info[i].fPosition_y = 50.0f; Player_Info[i].fPosition_z = 0.0f;
				vPlayer[i].Set_Position(DirectX::XMFLOAT3(0.0f, 50.0f, 0.0f));
				Player_Info[i].fLook_x = 0.0f; Player_Info[i].fLook_z = 1.0f;
				vPlayer[i].Set_Look_Vector(DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f));
				// 클라이언트 주소 변수에 추가 
				clientAddr[i] = clientaddr;

				EnterCriticalSection(&cs_for_logout);
				bPlayerLogout[i] = false;
				LeaveCriticalSection(&cs_for_logout);
				break;
			}
		}
	}
	else
	{
		// PlayerNumber 전달
		send(client_sock, (char*)&Current_Player_Count, sizeof(Current_Player_Count), 0);
		socket_vector.push_back(client_sock);
		Player_Info[Current_Player_Count].fPosition_x = 0.0f;
		Player_Info[Current_Player_Count].fPosition_y = 50.0f;
		Player_Info[Current_Player_Count].fPosition_z = 0.0f;
		vPlayer[Current_Player_Count].Set_Position(DirectX::XMFLOAT3(0.0f, 50.0f, 0.0f));
		// 클라이언트 주소 변수에 추가 
		clientAddr[Current_Player_Count] = clientaddr;
	}

	// 로그인 중 한명의 플레이어도 존재하지 않을 경우 쓰레드가 종료되는데
	// 그에 따른 추가 생성 
	if (Current_Player_Count == 0) {
		//플레이어 데이터 전송 쓰레드 미리 실행시키기
		HANDLE hThread = CreateThread(NULL, 0, SendPlayerDataToClient,
			(LPVOID)0, 0, NULL);
		CloseHandle(hThread);
	}

	// 플레이어 수 증가
	Current_Player_Count += 1;


	// 접속한 클라이언트 정보 출력
	char addr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
	printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
		addr, ntohs(clientaddr.sin_port));

	printf("총 플레이어 수 : %d\n", Current_Player_Count);
}


void InitGame()
{
	// 플레이어 위치 정보 초기화
	for (int i = 0; i < MAXPLAYERCOUNT; ++i) {
		Player_Info[i].fPosition_x = 0.0f;
		Player_Info[i].fPosition_y = -50.0f;
		Player_Info[i].fPosition_z = 0.0f;

		Player_Info[i].fLook_x = 0.0f;
		Player_Info[i].fLook_z = 1.0f;

		vPlayer[i].Set_Position(DirectX::XMFLOAT3(0.0f, -50.0f, 0.0f));
		vPlayer[i].Set_Look_Vector(DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f));
	}

	// 게임 시간 초기화
	g_startTime = g_prevTime = timeGetTime();
	remainingSeconds = GAMETIME-1;

	// 모든 소켓 초기화
	ClearAllSocket();

	// 현재 플레이어 수 조정
	Current_Player_Count = 0;

	// 클라이언트 주소 초기화
	for(int i=0; i<MAXPLAYERCOUNT; ++i)
	{
		clientAddr[i] = sockaddr_in{};
	}

	// 모든 플레이어 키 버퍼 초기화
	for(int i=0; i<MAXPLAYERCOUNT; ++i)
	{
		for(int j=0; j<256; ++j)	SetKeyBuffer(i, j, false);
	}

	// cs_for_logout 
	DeleteCriticalSection(&cs_for_logout);
	InitializeCriticalSection(&cs_for_logout);
	// cs_for_Cube
	DeleteCriticalSection(&cs_Cube);
	InitializeCriticalSection(&cs_Cube);

	// 로그아웃 변수 초기화
	for(int i=0; i<MAXPLAYERCOUNT; ++i)
	{
		bPlayerLogout[i] = false;
	}

}

void CreateClientKeyInputThread(SOCKET& KeyInput_listen_sock)
{
	SOCKET client_sock;
	struct sockaddr_in clientaddr;

	// accept()
	int addrlen = sizeof(clientaddr);
	client_sock = accept(KeyInput_listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
	if (client_sock == INVALID_SOCKET) {
		err_display("CreateClientKeyInputThread() - accept()");
		return;
	}
	

	HANDLE hThread = CreateThread(NULL, 0, ProcessClientKeyInput,
		(LPVOID)client_sock, 0, NULL);
	if (hThread == NULL) {  closesocket(client_sock); }
	else { CloseHandle(hThread); }
}
DWORD WINAPI ProcessClientKeyInput(LPVOID arg)
{
	SOCKET ClientKeyInputSocket = (SOCKET)arg;
	struct sockaddr_in clientaddr;

	struct KeyInput clientKeyInput{};
	int retval;
	while(1)
	{
		if (remainingSeconds <= 0) {
			break;
		}
		retval = recv(ClientKeyInputSocket, (char*)&clientKeyInput, sizeof(clientKeyInput), 0);
		if (retval == SOCKET_ERROR||(clientKeyInput.Key<0 || clientKeyInput.Key>256)) {
			break;
		}
		if(!clientKeyInput.KeyDown && (clientKeyInput.Key==27||clientKeyInput.Key==0))
		{
			break;
		}
		if(clientKeyInput.KeyDown)
		{
			SetKeyBuffer(clientKeyInput.PlayerNumber, clientKeyInput.Key, true);

		}else
		{
			SetKeyBuffer(clientKeyInput.PlayerNumber, clientKeyInput.Key, false);
		}
	}
	closesocket(ClientKeyInputSocket);
	return 0;
}

void CreateChatThread(SOCKET& chat_listen_sock)
{
	SOCKET client_sock;
	struct sockaddr_in clientaddr;

	// accept()
	int addrlen = sizeof(clientaddr);
	client_sock = accept(chat_listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
	if (client_sock == INVALID_SOCKET) {
		err_display("CreateClientKeyInputThread() - accept()");
		return;
	}
	if (socket_chat_vector.size() >= Current_Player_Count)		// 게임 시작 전 로그아웃 한 기록이 남아있는 것
	{
		for (int i = 0; i < socket_chat_vector.size(); ++i)
		{
			if (socket_chat_vector[i] == INVALID_SOCKET) {
				socket_chat_vector[i] = client_sock;
				break;
			}
		}
	}
	else
	{
		socket_chat_vector.push_back(client_sock);
	}

	HANDLE hThread = CreateThread(NULL, 0, ProcessEchoChat,
		(LPVOID)client_sock, 0, NULL);
	if (hThread == NULL) { closesocket(client_sock); }
	else { CloseHandle(hThread); }
}

DWORD WINAPI ProcessEchoChat(LPVOID arg)
{
	printf("채팅 쓰레드 시작\n");
	SOCKET echo_chat_socket = (SOCKET)arg;
	struct sockaddr_in clientaddr;

	ChatString chat_string;
	
	int retval;
	while (1)
	{
		if(echo_chat_socket != INVALID_SOCKET)
		{
			retval = recv(echo_chat_socket, (char*)&chat_string, sizeof(ChatString), MSG_WAITALL);
			if (retval == SOCKET_ERROR) {
				for (int i = 0; i < MAXPLAYERCOUNT; ++i)
				{
					if (socket_chat_vector[i] == echo_chat_socket) {
						return 0;
						//if (PlayerLogout(i)) return 0;
						
						break;
					}
				}
				break;
			}
		}
		
		
		int size = socket_chat_vector.size();
		for(int i =0; i<size; ++i)
		{
			if (socket_chat_vector[i] != INVALID_SOCKET)
			{
				retval = send(socket_chat_vector[i], (char*)&chat_string, sizeof(ChatString), 0);
				if(retval == SOCKET_ERROR)
				{
					return -1;
					//if(PlayerLogout(i)) return -1;
				}
				
			}
				
		}
	}
	return 0;
}
void CreateSendPlayerDataThread(SOCKET& senddata_listen_sock)
{
	SOCKET client_sock;
	struct sockaddr_in clientaddr;

	// accept()
	int addrlen = sizeof(clientaddr);
	client_sock = accept(senddata_listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
	if (client_sock == INVALID_SOCKET) {
		err_display("CreateSendPlayerDataThread() - accept()");
		return;
	}
	if (socket_SendPlayerData_vector.size() >= Current_Player_Count)		// 게임 시작 전 로그아웃 한 기록이 남아있는 것
	{
		for (int i = 0; i < socket_SendPlayerData_vector.size(); ++i)
		{
			if (socket_SendPlayerData_vector[i] == INVALID_SOCKET) {
				socket_SendPlayerData_vector[i] = client_sock;
				break;
			}
		}
	}
	else
	{
		socket_SendPlayerData_vector.push_back(client_sock);
	}

}

DWORD WINAPI SendPlayerDataToClient(LPVOID arg)
{
	printf("플레이어 정보 전송 및 룩벡터 받기 시작\n");
	//SOCKET SendPlayerDataSocket = (SOCKET)arg;
	//struct sockaddr_in clientaddr;

	int retval;
	struct Look_Data data;
	while (1) 
	{
		Sleep(16);
		int size = socket_SendPlayerData_vector.size();
		for (int i = 0; i < size; ++i) {
			if(socket_SendPlayerData_vector[i]!=INVALID_SOCKET)
			{
				retval = recv(socket_SendPlayerData_vector[i], (char*)&data, sizeof(struct Look_Data), MSG_WAITALL);
				if (retval == SOCKET_ERROR)
				{
					if(PlayerLogout(i)) return -1;
					continue;
				}
				Player_Info[data.PlayerNumber].fLook_x = data.fLook_x;
				Player_Info[data.PlayerNumber].fLook_z = data.fLook_z;
			}
			
		}

		// ElapsedTime 계산
		DWORD ElapsedTime = 0;
		DWORD CurrentTime = timeGetTime();
		if (g_prevTime == 0) ElapsedTime = CurrentTime - g_startTime;
		else ElapsedTime = CurrentTime - g_prevTime;
		g_prevTime = CurrentTime;
		float ElapsedTimeInSec = (float)ElapsedTime / 1000.0f;
		if (ElapsedTimeInSec > 0.1f)	ElapsedTimeInSec = 0.1f;

		//플레이어 이동로직 
		// 플레이어 충돌체크 및 움직임 갱신
		if(remainingSeconds <= GAMETIME)
		{
			
			for (int i = 0; i < MAXPLAYERCOUNT; ++i)
			{
				vPlayer[i].Set_Look_Vector(DirectX::XMFLOAT3(Player_Info[i].fLook_x, 0, Player_Info[i].fLook_z));
				vPlayer[i].Set_Right_Vector(vPlayer[i].Get_Look_Vector());

				vPlayer[i].Move(i, PLAYER_MOVE_DISTANCE * ElapsedTimeInSec, true);
				vPlayer[i].Update(i, ElapsedTimeInSec);
				vPlayer[i].Udt_N_Prcs_Collision(ppObjects, nObjects, i);

				Player_Info[i].fPosition_x = vPlayer[i].Get_Position().x;
				Player_Info[i].fPosition_y = vPlayer[i].Get_Position().y;
				Player_Info[i].fPosition_z = vPlayer[i].Get_Position().z;
			}
		}
		

		//플레이어 정보 모두 전송
		for(int i =0; i<socket_SendPlayerData_vector.size(); ++i)
		{
			if(socket_SendPlayerData_vector[i]!=INVALID_SOCKET)
			{
				retval = send(socket_SendPlayerData_vector[i], (char*)&Player_Info, sizeof(struct Player_Info) * MAXPLAYERCOUNT, 0);
				if (retval == SOCKET_ERROR) {
					if (PlayerLogout(i)) return -1;
					continue;
				}
			}
			
		}
	}
	return 0;
}



void CreateCubeThread(SOCKET& Cube_listen_sock)
{
	printf("큐브 쓰레드 시작\n");
	SOCKET client_sock;
	struct sockaddr_in clientaddr;

	// accept()
	int addrlen = sizeof(clientaddr);
	client_sock = accept(Cube_listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
	if (client_sock == INVALID_SOCKET) {
		err_display("CreateCubeThread() - accept()");
		return;
	}

	if (socket_Cube_vector.size() >= Current_Player_Count)		// 게임 시작 전 로그아웃 한 기록이 남아있는 것
	{
		for (int i = 0; i < socket_Cube_vector.size(); ++i)
		{
			if (socket_Cube_vector[i] == INVALID_SOCKET) {
				socket_Cube_vector[i] = client_sock;
				break;
			}
		}
	}
	else
	{
		socket_Cube_vector.push_back(client_sock);
	}

	// 현재 맵에 놓여있는 모든 큐브 정보 전송
	EnterCriticalSection(&cs_Cube);
	for(const auto cube : Total_Cube)
	{
		send(client_sock, (char*)&cube, sizeof(cube), 0);
	}
	LeaveCriticalSection(&cs_Cube);

	// 각 플레이어 마다 큐브 스레드 생성
	HANDLE hThread = CreateThread(NULL, 0, EchoClientRequestCube,
		(LPVOID)client_sock, 0, NULL);
	if (hThread == NULL) { closesocket(client_sock); }
	else { CloseHandle(hThread); }
}


DWORD WINAPI EchoClientRequestCube(LPVOID arg)
{
	SOCKET CubeSocket = (SOCKET)arg;
	struct sockaddr_in clientaddr;

	struct Cube_Info clientCubeInput;
	int retval;
	while (1)
	{
		// 큐브 리시브
		if (CubeSocket != INVALID_SOCKET)
		{
			retval = recv(CubeSocket, (char*)&clientCubeInput, sizeof(clientCubeInput), 0);
			if (retval == SOCKET_ERROR) {
				for (int i = 0; i < socket_Cube_vector.size(); ++i)
				{
					if (socket_Cube_vector[i] == CubeSocket)
					{
						return -1;
						if (PlayerLogout(i)) return -1;
						break;
					}
				}
				break;
			}
		}

		if (remainingSeconds <= GAMETIME)
		{

			
			// Cube Add
			if (clientCubeInput.AddorDelete)
			{
				// 여기서 큐브와 사람 충돌체크
				if (Check_Add_Cube(clientCubeInput))
				{
				}
				// 가능시에만 각 클라에게 큐브 정보 send
				else
				{
					// add to Total_Cube
					EnterCriticalSection(&cs_Cube);
					Total_Cube.push_back(clientCubeInput);
					LeaveCriticalSection(&cs_Cube);

					// add to Cube Object
					CObject* pObject = new CObject();
					pObject->Set_Position(clientCubeInput.fPosition_x, clientCubeInput.fPosition_y, clientCubeInput.fPosition_z);
					ppObjects[nObjects++] = pObject;

					// 큐브 send to every client
					for (int i = 0; i < socket_Cube_vector.size(); ++i)
					{
						if (socket_Cube_vector[i] != INVALID_SOCKET)
						{
							int retval = send(socket_Cube_vector[i], (char*)&clientCubeInput, sizeof(clientCubeInput), 0);
							if (retval == SOCKET_ERROR) {
								return -1;
								//if (PlayerLogout(i)) return -1;
								break;
							}
						}
					}

				}
			}
			// Cube Delete
			else
			{
				Delete_Cube(clientCubeInput);
			}
		}
	}
	return 0;

}

DWORD WINAPI Send_Game_Time(LPVOID arg) {
	remainingSeconds = GAMETIME + 3;
	std::cout << "시간 쓰레드 시작" << std::endl;
	// 시간 데이터 보내기
	while (true)
	{
		if (Current_Player_Count == 0) {
			std::cout << "시간 쓰레드 종료" << std::endl;
			return 0;
		}
		// 남은 시간이 0보다 크거나 같으면 클라이언트로 시간 업데이트 및 전송
		if (remainingSeconds >= 0) 
		{
			for (int i = 0; i < socket_vector.size(); ++i)
			{
				if (socket_vector[i])
				{
					int retval = send(socket_vector[i], (char*)&remainingSeconds, sizeof(int), 0);
					if (retval == SOCKET_ERROR) {
						if (PlayerLogout(i)) return 0;
						continue;
					}
				}
			}
			std::cout << "남은 게임 시간: " << remainingSeconds << " seconds" << std::endl;
		}
		
		// 게임 종료 이벤트 
		else
		{
			std::array<int, MAXPLAYERCOUNT> player_cube_count {};
			EnterCriticalSection(&cs_Cube);
			for (const auto cube : Total_Cube)
			{
				if (fabs(cube.fColor_r - 1.0f) < FLT_EPSILON) ++player_cube_count[0];
				if (fabs(cube.fColor_g - 1.0f) < FLT_EPSILON) ++player_cube_count[1];
				if (fabs(cube.fColor_b - 1.0f) < FLT_EPSILON) ++player_cube_count[2];
			}
			LeaveCriticalSection(&cs_Cube);
			int max = 0;
			int number_max = 0;
			for (int i = 0; i < 3; i++) {
				if (player_cube_count[i] > max)
				{
					max = player_cube_count[i];
					number_max = i;
				}
			}
			switch (number_max) {
			case 0:
				number_max = -1;
				break;
			case 1:
				number_max = -2;
				break;
			case 2:
				number_max = -3;
				break;
			}
			for (int i = 0; i < socket_vector.size(); ++i)
			{
				if (socket_vector[i] != INVALID_SOCKET)
				{
					int retval = send(socket_vector[i], (char*)&number_max, sizeof(int), 0);
					if (retval == SOCKET_ERROR) {
						if (PlayerLogout(i)) return 0;
						continue;
					}
					std::cout << i << "큐브 설치 전송됌" << std::endl;
				}
			}
			break;

		}
		
		Sleep(1000);	// 1초에 한번씩 보내도록 쓰레드를 sleep
		--remainingSeconds;
	}
	std::cout << "시간 쓰레드 종료" << std::endl;
	Sleep(1000);
	for (int i = 0; i < MAXPLAYERCOUNT; ++i) {
		PlayerLogout(i);
	}
	return 0;
}

// 모든 플레이어랑 설치하려는 큐브와 미리 충돌체크하여 return
bool Check_Add_Cube(Cube_Info cube)
{
	CObject* CheckCollsion_Cube = NULL;
	CheckCollsion_Cube = new CObject();
	CheckCollsion_Cube->Set_Position(cube.fPosition_x, cube.fPosition_y, cube.fPosition_z);
	for (int i = 0; i < MAXPLAYERCOUNT; i++)
	{
		bool checkCube = vPlayer[i].Check_Collision_Add_Cube(CheckCollsion_Cube, i);
		if (checkCube)
		{
			return true;
		}
	}
	return false;

}


bool CompareXMFLOAT3(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b) {
	return (a.x == b.x && a.y == b.y && a.z == b.z);
}


void ClearAllSocket()
{
	/* 키 인풋 소켓의 경우 다른 클라들에게 정보를 전송할 필요가 없어 해당 쓰레드에서 지역변수로 선언되어 초기화 할 필요가 없음. (CloseSocket은 정상 진행) */
	for(auto p = socket_vector.begin(); p!=socket_vector.end(); ++p)	if (*p != INVALID_SOCKET) closesocket(*p);
	for (auto p = socket_Cube_vector.begin(); p != socket_Cube_vector.end(); ++p)	if (*p != INVALID_SOCKET) closesocket(*p);
	for (auto p = socket_SendPlayerData_vector.begin(); p != socket_SendPlayerData_vector.end(); ++p)	if (*p != INVALID_SOCKET) closesocket(*p);
	for (auto p = socket_chat_vector.begin(); p != socket_chat_vector.end(); ++p)	if (*p != INVALID_SOCKET) closesocket(*p);

	socket_vector.clear();
	socket_Cube_vector.clear();
	socket_SendPlayerData_vector.clear();
	socket_chat_vector.clear();
}



bool PlayerLogout(int playerNumber)
{
	
	// 소켓 정리
	EnterCriticalSection(&cs_for_logout);
	if (socket_vector[playerNumber] != INVALID_SOCKET && !bPlayerLogout[playerNumber]) {
		mtx.lock();
		bPlayerLogout[playerNumber] = true;
		closesocket(socket_vector[playerNumber]);
		//if (socket_Cube_vector[playerNumber] != INVALID_SOCKET)	
		closesocket(socket_Cube_vector[playerNumber]);
		//if (socket_SendPlayerData_vector[playerNumber] != INVALID_SOCKET)	
		closesocket(socket_SendPlayerData_vector[playerNumber]);
		//if (socket_chat_vector[playerNumber] != INVALID_SOCKET)	
		closesocket(socket_chat_vector[playerNumber]);
		socket_vector[playerNumber] = INVALID_SOCKET;
		socket_Cube_vector[playerNumber] = INVALID_SOCKET;
		socket_SendPlayerData_vector[playerNumber] = INVALID_SOCKET;
		socket_chat_vector[playerNumber] = INVALID_SOCKET;
		mtx.unlock();


		// 현재 플레이어 수 줄이기
		Current_Player_Count -= 1;

		Player_Info[playerNumber].fPosition_x = 0.0f; Player_Info[playerNumber].fPosition_y = -50.0f; Player_Info[playerNumber].fPosition_z = 0.0f;
		vPlayer[playerNumber].Set_Position(DirectX::XMFLOAT3(0.0f, -50.0f, 0.0f));
		Player_Info[playerNumber].fLook_x = 0.0f; Player_Info[playerNumber].fLook_z = 1.0f;
		vPlayer[playerNumber].Set_Look_Vector(DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f));
		
	}
	LeaveCriticalSection(&cs_for_logout);
	if (Current_Player_Count == 0) return true;
	else return false;
	
}

void CreateListenSockets(SOCKET& login_listen_sock, SOCKET& KeyInput_listen_sock, SOCKET& Cube_listen_sock, SOCKET& send_playerdata_listen_sock, SOCKET& echo_chat_listen_sock)
{
	//----------------로그인 소켓 만드는 과정----------------   //TODO: 아래꺼랑 합쳐서 함수화 처리 예정
	int retval;

	// 윈속 초기화
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return;

	// 소켓 생성(플레이어 접속 체크 소켓)
	login_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (login_listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(login_listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(login_listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	//---------------- 소켓 만드는 과정(키 인풋 소켓)----------------
	// 소켓 생성(키 인풋 리슨 소켓)
	KeyInput_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (KeyInput_listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind() - KEYINPUTSERVERPORT 로 바인드,
	struct sockaddr_in serveraddr_keyinput;
	memset(&serveraddr_keyinput, 0, sizeof(serveraddr_keyinput));
	serveraddr_keyinput.sin_family = AF_INET;
	serveraddr_keyinput.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr_keyinput.sin_port = htons(KEYINPUTSERVERPORT);

	if (bind(KeyInput_listen_sock, (struct sockaddr*)&serveraddr_keyinput, sizeof(serveraddr_keyinput))
		== SOCKET_ERROR) err_quit("CreateClientKeyInputThread() - bind()");

	// listen()
	if (listen(KeyInput_listen_sock, SOMAXCONN)
		== SOCKET_ERROR) err_quit("CreateClientKeyInputThread() - listen()");


	//---------------- 소켓 만드는 과정(큐브 소켓)----------------
	// 소켓 생성(큐브 리슨 소켓)
	Cube_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (Cube_listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr_Cube;
	memset(&serveraddr_Cube, 0, sizeof(serveraddr_Cube));
	serveraddr_Cube.sin_family = AF_INET;
	serveraddr_Cube.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr_Cube.sin_port = htons(CUBESERVERPORT);

	if (bind(Cube_listen_sock, (struct sockaddr*)&serveraddr_Cube, sizeof(serveraddr_Cube))
		== SOCKET_ERROR) err_quit("CreateCubeThread() - bind()");

	// listen()
	if (listen(Cube_listen_sock, SOMAXCONN)
		== SOCKET_ERROR) err_quit("CreateCubeThread() - listen()");

	//---------------- 소켓 만드는 과정(플레이어 데이터 전송 소켓)----------------
	// 소켓 생성(플레이어 데이터 전송 소켓)
	send_playerdata_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (send_playerdata_listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr_sendPlayerData;
	memset(&serveraddr_sendPlayerData, 0, sizeof(serveraddr_sendPlayerData));
	serveraddr_sendPlayerData.sin_family = AF_INET;
	serveraddr_sendPlayerData.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr_sendPlayerData.sin_port = htons(SENDPLAYERDATAPORT);

	if (bind(send_playerdata_listen_sock, (struct sockaddr*)&serveraddr_sendPlayerData, sizeof(serveraddr_sendPlayerData))
		== SOCKET_ERROR) err_quit("bind()");

	// listen()
	if (listen(send_playerdata_listen_sock, SOMAXCONN)
		== SOCKET_ERROR) err_quit("listen()");

	//---------------- 소켓 만드는 과정(채팅 소켓)----------------
	// 소켓 생성(채팅 소켓)
	echo_chat_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (echo_chat_listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr_chat;
	memset(&serveraddr_chat, 0, sizeof(serveraddr_chat));
	serveraddr_chat.sin_family = AF_INET;
	serveraddr_chat.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr_chat.sin_port = htons(ECHOCHATDATAPORT);

	if (bind(echo_chat_listen_sock, (struct sockaddr*)&serveraddr_chat, sizeof(serveraddr_chat))
		== SOCKET_ERROR) err_quit("bind()");

	// listen()
	if (listen(echo_chat_listen_sock, SOMAXCONN)
		== SOCKET_ERROR) err_quit("listen()");

}

void LoginPlayer(SOCKET& login_listen, SOCKET& keyinput_listen, SOCKET& cube_listen, SOCKET& playerdata_listen, SOCKET& chat_listen)
{
	while (Current_Player_Count != MAXPLAYERCOUNT) {
		ConnectAndAddPlayer(login_listen);
		CreateClientKeyInputThread(keyinput_listen);
		CreateCubeThread(cube_listen);
		CreateSendPlayerDataThread(playerdata_listen);
		CreateChatThread(chat_listen);
	}

	// 게임 시작시 기존 큐브 모두 삭제 및 기본 바닥 큐브 재설치
	Request_Delete_All_Cube();
	Release_Floor_Cube_Object();
	Set_Floor_Cube_Object();

	Player_Info[0].fPosition_x = 75.0f; Player_Info[0].fPosition_y = 50.0f; Player_Info[0].fPosition_z = 0.0f;
	Player_Info[1].fPosition_x = 0.0f; Player_Info[1].fPosition_y = 50.0f; Player_Info[1].fPosition_z = 0.0f;
	Player_Info[2].fPosition_x = 0.0f; Player_Info[2].fPosition_y = 50.0f; Player_Info[2].fPosition_z = -75.0f;
	vPlayer[0].Set_Position(DirectX::XMFLOAT3(75.0f, 50.0f, 0.0f));
	vPlayer[1].Set_Position(DirectX::XMFLOAT3(0.0f,50.0f,0.0f));
	vPlayer[2].Set_Position(DirectX::XMFLOAT3(0.0f,50.0f,-75.0f));

	// 소켓 닫기
	closesocket(login_listen);
	closesocket(keyinput_listen);
	closesocket(cube_listen);
	closesocket(playerdata_listen);
	closesocket(chat_listen);
}

// 바닥 큐브 set
void Set_Floor_Cube_Object()
{
	nObjects = (CUBE_INIT_RING_NUMBER * 2 + 1) * (CUBE_INIT_RING_NUMBER * 2 + 1);
	ppObjects = new CObject * [CUBE_MAX_NUMBER] { NULL };

	int c_i = 0;
	CObject* pObject = NULL;

	for (int x = -CUBE_INIT_RING_NUMBER; x <= CUBE_INIT_RING_NUMBER; ++x) {
		for (int z = -CUBE_INIT_RING_NUMBER; z <= CUBE_INIT_RING_NUMBER; ++z) {
			pObject = new CObject();
			pObject->Set_Position(CUBE_WIDTH * x, 0.0f, CUBE_WIDTH * z);
			ppObjects[c_i++] = pObject;
		}
	}
}

// 바닥 및 모든 큐브 관련 변수 초기화
void Release_Floor_Cube_Object()
{
	// 큐브 데이터 초기화
	Total_Cube.clear();

	if (nObjects)	nObjects = 0;
	if (ppObjects != NULL)
	{
		ppObjects = NULL;
	}
}

// 큐브를 삭제하기 위한 함수
void Delete_Cube(Cube_Info clientCubeInput)
{
	EnterCriticalSection(&cs_Cube);
	auto it = std::find_if(Total_Cube.begin(), Total_Cube.end(), [&](const Cube_Info& value) {
		return CompareXMFLOAT3(DirectX::XMFLOAT3(value.fPosition_x, value.fPosition_y, value.fPosition_z),
		DirectX::XMFLOAT3(clientCubeInput.fPosition_x, clientCubeInput.fPosition_y, clientCubeInput.fPosition_z));
		});

	if (it != Total_Cube.end())
	{
		// 큐브 값은 찾은 경우
		Total_Cube.erase(it);

		// delete to Cube Object
		CObject* pSelected_Object = NULL;
		int nSelected_Index = 0;
		for (int i = 0; i < nObjects; ++i) {
			if (CompareXMFLOAT3(ppObjects[i]->Get_Position(), DirectX::XMFLOAT3(clientCubeInput.fPosition_x, clientCubeInput.fPosition_y, clientCubeInput.fPosition_z)))
			{
				pSelected_Object = ppObjects[i];
				nSelected_Index = i;
			}
		}
		if (pSelected_Object)
		{
			ppObjects[nSelected_Index] = NULL;

			if (nSelected_Index != nObjects - 1) {
				ppObjects[nSelected_Index] = ppObjects[nObjects - 1];
			}
			--nObjects;
		}

		for (int i = 0; i < socket_Cube_vector.size(); ++i)
		{
			if (socket_Cube_vector[i] != INVALID_SOCKET)
			{
				int retval = send(socket_Cube_vector[i], (char*)&clientCubeInput, sizeof(clientCubeInput), 0);
				if (retval == SOCKET_ERROR) {
					break;
				}
			}
		}
	}
	else {
		// 큐브 값을 찾지 못한 경우
		std::cout << "Delete Cube Value not found!" << std::endl;
	}
	LeaveCriticalSection(&cs_Cube);
}

// 모든 큐브 삭제하는 코드
void Request_Delete_All_Cube() 
{
	// 현재 맵에 놓여있는 모든 큐브 정보 삭제로 바꿔서 전송
	/*EnterCriticalSection(&cs_Cube);
	for (auto socket : socket_Cube_vector) {
		for (auto cube : Total_Cube)
		{
			cube.AddorDelete = false;
			send(socket, (char*)&cube, sizeof(cube), 0);
		}
	}
	LeaveCriticalSection(&cs_Cube);*/

	// 큐브 데이터 초기화
}