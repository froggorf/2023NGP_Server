#include "Common.h"
#include "Global.h"
#include "ServerData.h"
#include "ClientKeyInput.h"
#include "ProcessClientInput.h"
#include "array"

// Timer 관련
#include <Mmsystem.h>
#pragma comment(lib, "winmm.lib")


// 함수
void ConnectAndAddPlayer(SOCKET&);
void InitGame();						// 게임 데이터 초기화 부분, 재시작 시 다시 호출하여 실행할 수 있도록 구현 예정
void CreateClientKeyInputThread(SOCKET& KeyInput_listen_sock);
VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime); 	// 시간 업데이트 함수
void Send_Game_Time();
int remainingSeconds = GAMETIME; // 5분을 초로 환산

HWND timerHWND;

// 키 인풋 받아 처리하는 쓰레드
DWORD WINAPI ProcessClientKeyInput(LPVOID arg);

// 플레이어 정보 전송하는 쓰레드
void CreateSendPlayerDataThread(SOCKET& senddata_listen_sock);
DWORD WINAPI SendPlayerDataToClient(LPVOID arg);

// 채팅 서버 생성 쓰레드
void CreateChatThread(SOCKET& chat_listen_sock);
DWORD WINAPI ProcessEchoChat(LPVOID arg);

void CreateCubeThread(SOCKET& Cube_listen_sock);
DWORD WINAPI EchoClientRequestCube(LPVOID arg);
bool Check_Add_Cube(Cube_Info cube);

bool CompareXMFLOAT3(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b);

// 윈속 변수
WSADATA wsa;

// ElapsedTime 관련
DWORD g_startTime;
DWORD g_prevTime;

// 게임 초기화 시 모든 소켓 정보 초기화하는 함수
void ClearAllSocket();

// 플레이어가 게임 진행중 게임을 종료해 서버에서 나갔을 때 함수
bool PlayerLogout(int playerNumber);
CRITICAL_SECTION cs_for_logout;
bool bPlayerLogout[MAXPLAYERCOUNT] = { false, };


int main(int argc, char *argv[])
{
	int retval;

	

	while(true)
	{
		nObjects = (CUBE_INIT_RING_NUMBER * 2 + 1) * (CUBE_INIT_RING_NUMBER * 2 + 1);
		ppObjects = new CObject * [CUBE_MAX_NUMBER] { NULL };

		int c_i = 0;
		CObject* pObject = NULL;

		for (int x = -CUBE_INIT_RING_NUMBER; x <= CUBE_INIT_RING_NUMBER; ++x) {
			for (int z = -CUBE_INIT_RING_NUMBER; z <= CUBE_INIT_RING_NUMBER; ++z) {
				pObject = new CObject();
				pObject->Set_Position(CUBE_WIDTH * x, 0.0f, CUBE_WIDTH * z);
				//pObject->Set_Color(CUBE_DEFAULT_COLOR, CUBE_DEFAULT_COLOR, CUBE_DEFAULT_COLOR, 0.0f);
				ppObjects[c_i++] = pObject;
			}
		}

		
		//----------------로그인 소켓 만드는 과정----------------   //TODO: 아래꺼랑 합쳐서 함수화 처리 예정
		// 윈속 초기화
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
			return 1;

		// 소켓 생성(플레이어 접속 체크 소켓)
		SOCKET login_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (login_listen_sock == INVALID_SOCKET) err_quit("socket()");

		// bind()
		struct sockaddr_in serveraddr;
		memset(&serveraddr, 0, sizeof(serveraddr));
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
		serveraddr.sin_port = htons(SERVERPORT);
		retval = bind(login_listen_sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
		if (retval == SOCKET_ERROR) err_quit("bind()");

		// listen()
		retval = listen(login_listen_sock, SOMAXCONN);
		if (retval == SOCKET_ERROR) err_quit("listen()");

		//---------------- 소켓 만드는 과정(키 인풋 소켓)----------------
		// 소켓 생성(키 인풋 리슨 소켓)
		SOCKET KeyInput_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
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
		SOCKET Cube_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
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
		SOCKET send_playerdata_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
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
		SOCKET echo_chat_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
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

		//-----------------
		// 게임 데이터 초기화
		InitGame();
		
		//플레이어 데이터 전송 쓰레드 및 룩벡터 전송받는 쓰레드 미리 실행시키기
		HANDLE hThread = CreateThread(NULL, 0, SendPlayerDataToClient,
			(LPVOID)0, 0, NULL);
		CloseHandle(hThread);

		// 플레이어 지정한 수 인원 접속시키기
		while (Current_Player_Count != MAXPLAYERCOUNT) {
			ConnectAndAddPlayer(login_listen_sock);
			CreateClientKeyInputThread(KeyInput_listen_sock);
			CreateCubeThread(Cube_listen_sock);
			CreateSendPlayerDataThread(send_playerdata_listen_sock);
			CreateChatThread(echo_chat_listen_sock);
		}
		// 소켓 닫기
		closesocket(login_listen_sock);
		closesocket(KeyInput_listen_sock);
		closesocket(Cube_listen_sock);
		closesocket(send_playerdata_listen_sock);
		closesocket(echo_chat_listen_sock);



		// TODO: 게임 시작 되므로 게임 시작 관련 스레드 제작 예정

		// 타이머 초기화
		SetTimer(timerHWND, TIMER_ID, 1000, TimerProc); // 1000ms(1초)마다 타이머 호출
		std::cout << "타이머 시작 - " << remainingSeconds << std::endl;

		// 플레이어의 키 인풋 정보를 받는 쓰레드 생성

		// TODO: while문으로 main 쓰레드에서는 중력, 충돌체크 및 시간 전송 등이 진행되도록 구현 예정
		bool bGame = true;
		while (bGame) {
			// 시간 처리를 위한 메세지 루프
			MSG msg;
			while (GetMessage(&msg, NULL, 0, 0)) {
				// 서버 종료
				if (Current_Player_Count == 0)
				{
					printf("모든 플레이어 종료를 확인함\n");
					bGame = false;
					break;
				}
				

				TranslateMessage(&msg);
				DispatchMessage(&msg);
				
			}
			

		}

		printf("서버 종료를 확인함 다시 while 돌리면 됨\n");
		InitGame();
		// TODO: EndGame() 로직 만들기.


	}
	// 소켓 닫기
	//closesocket(listen_sock);


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

	// PlayerNumber 전달
	send(client_sock, (char*)&Current_Player_Count, sizeof(Current_Player_Count), 0);
	// 시간 통신용 소켓 벡터에 저장
	socket_vector.push_back(client_sock);
	// 클라이언트 주소 변수에 추가 
	clientAddr[Current_Player_Count] = clientaddr;

	Player_Info[Current_Player_Count].fPosition_x = 0.0f;
	Player_Info[Current_Player_Count].fPosition_y = 50.0f;
	Player_Info[Current_Player_Count].fPosition_z = 0.0f;

	vPlayer[Current_Player_Count].Set_Position(DirectX::XMFLOAT3(0.0f, 50.0f, 0.0f));

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
	remainingSeconds = GAMETIME;

	// 큐브 데이터 초기화
	Total_Cube.clear();

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

	// 로그아웃 변수 초기화
	for(int i=0; i<MAXPLAYERCOUNT; ++i)
	{
		bPlayerLogout[i] = false;
	}

	KillTimer(timerHWND, TIMER_ID);
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
	printf("키 인풋 쓰레드 시작\n");
	SOCKET ClientKeyInputSocket = (SOCKET)arg;
	struct sockaddr_in clientaddr;

	struct KeyInput clientKeyInput{};
	int retval;
	while(1)
	{
		
		retval = recv(ClientKeyInputSocket, (char*)&clientKeyInput, sizeof(clientKeyInput), 0);
		if (retval == SOCKET_ERROR||(clientKeyInput.Key<0 || clientKeyInput.Key>256)) {
			closesocket(ClientKeyInputSocket);
			break;
		}
		if(!clientKeyInput.KeyDown && (clientKeyInput.Key==27||clientKeyInput.Key==0))
		{
			//종료한 상황으로 확인됨
			closesocket(ClientKeyInputSocket);
			break;
		}
		printf("%d 플레이어가 %d 버튼 ", clientKeyInput.PlayerNumber, clientKeyInput.Key);
		if(clientKeyInput.KeyDown)
		{
			SetKeyBuffer(clientKeyInput.PlayerNumber, clientKeyInput.Key, true);
			printf("누름\n");

		}else
		{
			SetKeyBuffer(clientKeyInput.PlayerNumber, clientKeyInput.Key, false);
			printf("뗌\n");
		}
	}

	printf("클라이언트 키 인풋은 잘 지워짐\n");
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

	socket_chat_vector.push_back(client_sock);

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
						if (PlayerLogout(i)) return 0;
						
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
					if(PlayerLogout(i)) return -1;
				}
				
			}
				
		}
	}
	printf("채팅 쓰레드는 잘 지워짐");
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
	socket_SendPlayerData_vector.push_back(client_sock);


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
		
		//플레이어 이동로직 
		// 플레이어 충돌체크 및 움직임 갱신
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
	socket_Cube_vector.push_back(client_sock);

	// 현재 맵에 놓여있는 모든 큐브 정보 전송
	for(const auto cube : Total_Cube)
	{
		send(client_sock, (char*)&cube, sizeof(cube), 0);
	}

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
		if(CubeSocket!=INVALID_SOCKET)
		{
			retval = recv(CubeSocket, (char*)&clientCubeInput, sizeof(clientCubeInput), 0);
			if (retval == SOCKET_ERROR) {
				for(int i=0; i<socket_Cube_vector.size();++i)
				{
					if(socket_Cube_vector[i]==CubeSocket)
					{
						if (PlayerLogout(i)) return -1;
						break;
					}
				}
				break;
			}
		}
		
		printf("Cube Position - %.2f, %.2f, %.2f\n", clientCubeInput.fPosition_x, clientCubeInput.fPosition_y, clientCubeInput.fPosition_z);
		printf("Cube Color - %.2f, %.2f, %.2f\n", clientCubeInput.fColor_r, clientCubeInput.fColor_g, clientCubeInput.fColor_b);
		printf("Cube Add or Delete - %s\n", clientCubeInput.AddorDelete ? "Add" : "Delete");
		
		// Cube Add
		if (clientCubeInput.AddorDelete)	
		{
			// 여기서 큐브와 사람 충돌체크
			if (Check_Add_Cube(clientCubeInput))
			{
				printf("큐브 설치 불가능\n");
			}
			// 가능시에만 각 클라에게 큐브 정보 send
			else
			{
				printf("큐브 설치 가능\n");
				// add to Total_Cube
				Total_Cube.push_back(clientCubeInput);
				// add to Cube Object
				CObject* pObject = new CObject();
				pObject->Set_Position(clientCubeInput.fPosition_x, clientCubeInput.fPosition_y, clientCubeInput.fPosition_z);
				ppObjects[nObjects++] = pObject;

				// 큐브 send to every client
				for(int i=0; i<socket_Cube_vector.size(); ++i)
				{
					if(socket_Cube_vector[i]!=INVALID_SOCKET)
					{
						int retval = send(socket_Cube_vector[i], (char*)&clientCubeInput, sizeof(clientCubeInput), 0);
						if (retval == SOCKET_ERROR) {
							if (PlayerLogout(i)) return -1;
							break;
						}
						std::cout << "Sending Add cube_info to the client" << std::endl;
					}
				}
				
			}
		}
		// Cube Delete
		else
		{
			auto it = std::find_if(Total_Cube.begin(), Total_Cube.end(), [&](const Cube_Info& value) {
				return CompareXMFLOAT3(DirectX::XMFLOAT3(value.fPosition_x, value.fPosition_y, value.fPosition_z),
				DirectX::XMFLOAT3(clientCubeInput.fPosition_x, clientCubeInput.fPosition_y, clientCubeInput.fPosition_z));
				});

			if (it != Total_Cube.end()) 
			{
				// 큐브 값은 찾은 경우
				printf("큐브 삭제 가능\n");
				Total_Cube.erase(it);

				// delete to Cube Object
				CObject* pSelected_Object = NULL;
				int nSelected_Index = 0;
				for (int i = 0; i < nObjects; ++i) {
					if (CompareXMFLOAT3(ppObjects[i]->Get_Position(), DirectX::XMFLOAT3(clientCubeInput.fPosition_x, clientCubeInput.fPosition_y, clientCubeInput.fPosition_z)) )
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
							if (PlayerLogout(i)) return -1;
							break;
						}
						std::cout << "Sending Delete cube_info to the client" << std::endl;
					}
				}
			}
			else {
				// 큐브 값을 찾지 못한 경우
				std::cout << "Delete Cube Value not found!" << std::endl;
			}
		}
	}
	return 0;
}


// 타이머 콜백 함수
VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
	printf("시간 타이머 함수 실행중\n");
	// 시간 업데이트
	remainingSeconds--;
	// 남은 시간이 0보다 크거나 같으면 클라이언트로 시간 업데이트 및 전송
	if (remainingSeconds >= 0) {
		Send_Game_Time();
	}
	else {
		// 게임 종료 이벤트 여기에 추가
		std::array<int, MAXPLAYERCOUNT> player_cube_count;
		for(const auto cube: Total_Cube)
		{
			if (cube.fColor_r - 1.0f < FLT_EPSILON) ++player_cube_count[0];
			if (cube.fColor_g - 1.0f < FLT_EPSILON) ++player_cube_count[1];
			if (cube.fColor_b - 1.0f < FLT_EPSILON) ++player_cube_count[2];
		}
		for(int i=0; i<socket_vector.size(); ++i)
		{
			if(socket_vector[i] !=INVALID_SOCKET)
			{
				int retval = send(socket_vector[i], (char*)&player_cube_count, sizeof(int) * MAXPLAYERCOUNT, 0);
				if (retval == SOCKET_ERROR)
				{
					if (PlayerLogout(i)) return;
					if(Current_Player_Count==0) KillTimer(timerHWND, TIMER_ID);
					break;
				}
			}
		}
		// 타이머 종료
		KillTimer(timerHWND, TIMER_ID);
		//for (int i = 0; i < MAXPLAYERCOUNT; ++i)	closesocket(socket_vector[i]);		// 시간 소켓 close
	}
}


void Send_Game_Time() {
	// 시간 데이터 보내기
	for(int i=0; i<socket_vector.size(); ++i)
	{
		if(socket_vector[i])
		{
			int retval = send(socket_vector[i], (char*)&remainingSeconds, sizeof(int), 0);
			if (retval == SOCKET_ERROR) {
				if(PlayerLogout(i)) return;
				continue;
			}
		}
	}
	std::cout << "Sending time to the client: " << remainingSeconds << " seconds" << std::endl;
}

bool Check_Add_Cube(Cube_Info cube)
{
	for (int i = 0; i < MAXPLAYERCOUNT; i++)
	{
		// X, Z 축에 대해 겹치는지 확인
		float xDiff = Player_Info[i].fPosition_x - cube.fPosition_x;
		float zDiff = Player_Info[i].fPosition_z - cube.fPosition_z;

		bool isXInRange = (xDiff >= -5.0f) && (xDiff <= 5.0f);
		bool isZInRange = (zDiff >= -5.0f) && (zDiff <= 5.0f);

		// Y 축에 대해 높이 확인
		bool isYInRange = (Player_Info[i].fPosition_y >= cube.fPosition_y) && (Player_Info[i].fPosition_y <= cube.fPosition_y + 10.0f);

		printf("xDiff : %.2f, zDiff : %.2f, bool Y : %d\n", xDiff, zDiff, isYInRange);
		// 모든 축에서 조건이 만족하는지 확인하여 블록 안에 플레이어가 있는지 반환
		return isXInRange && isZInRange && isYInRange;
		
	}
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
		bPlayerLogout[playerNumber] = true;
		printf(" %d 번째 플레이어가 로그아웃 하였습니다.\n ", playerNumber);
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

		//socket_vector.erase(socket_vector.begin() + playerNumber);
		//socket_Cube_vector.erase(socket_Cube_vector.begin() + playerNumber);
		//socket_SendPlayerData_vector.erase(socket_SendPlayerData_vector.begin() + playerNumber);
		//socket_chat_vector.erase(socket_chat_vector.begin() + playerNumber);

		// 현재 플레이어 수 줄이기
		Current_Player_Count -= 1;
		printf("%d 명의 플레이어만 남음\n",Current_Player_Count);
		
	}
	LeaveCriticalSection(&cs_for_logout);
	if (Current_Player_Count == 0) return true;
	else return false;
	
}
