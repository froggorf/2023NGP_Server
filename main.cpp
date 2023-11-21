#include "Common.h"
#include "Global.h"
#include "ServerData.h"
#include "ClientKeyInput.h"
#include "ProcessClientInput.h"



// 함수
void ConnectAndAddPlayer(SOCKET&);
void InitGame();						// 게임 데이터 초기화 부분, 재시작 시 다시 호출하여 실행할 수 있도록 구현 예정
void CreateClientKeyInputThread(SOCKET& KeyInput_listen_sock);
VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime); 	// 시간 업데이트 함수
void Send_Game_Time();
void SendPlayerLocationToAllClient();
int remainingSeconds = MAX_MIN * 60; // 5분을 초로 환산

// 키 인풋 받아 처리하는 쓰레드
DWORD WINAPI ProcessClientKeyInput(LPVOID arg);

// 플레이어 정보 전송하는 쓰레드
void CreateSendPlayerDataThread(SOCKET& senddata_listen_sock);
DWORD WINAPI SendPlayerDataToClient(LPVOID arg);

void CreateCubeThread(SOCKET& Cube_listen_sock);
DWORD WINAPI EchoClientRequestCube(LPVOID arg);

// 윈속 변수
WSADATA wsa;


int main(int argc, char *argv[])
{
	int retval;

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
	if (KeyInput_listen_sock == INVALID_SOCKET) err_quit("socket()");

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

	//-----------------
	// 게임 데이터 초기화
	InitGame();


	// 플레이어 지정한 수 인원 접속시키기
	while (Current_Player_Count != MAXPLAYERCOUNT) {
		ConnectAndAddPlayer(login_listen_sock);
		CreateClientKeyInputThread(KeyInput_listen_sock);
		CreateCubeThread(Cube_listen_sock);
		CreateSendPlayerDataThread(send_playerdata_listen_sock);
	}

	// 소켓 닫기
	closesocket(login_listen_sock);



	// TODO: 게임 시작 되므로 게임 시작 관련 스레드 제작 예정

	// 타이머 초기화
	SetTimer(NULL, TIMER_ID, 1000, TimerProc); // 1000ms(1초)마다 타이머 호출
	std::cout << "타이머 시작 - " << remainingSeconds << std::endl;

	// 플레이어의 키 인풋 정보를 받는 쓰레드 생성

	// TODO: while문으로 main 쓰레드에서는 중력, 충돌체크 및 시간 전송 등이 진행되도록 구현 예정
	while (true) {


		// 시간 처리를 위한 메세지 루프
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			//ProcessClientInput();
		}

		SendPlayerLocationToAllClient();
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

	// 플레이어 수 증가
	Current_Player_Count += 1;

	//// 새로운 플레이어 추가
	//struct Player_Info* newPlayer = new struct Player_Info();
	//Player_Info.push_back(newPlayer);
	// 플레이어 위치 갱신
	
	

	// 접속한 클라이언트 정보 출력
	char addr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
	printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
		addr, ntohs(clientaddr.sin_port));

	printf("총 플레이어 수 : %d\n", Current_Player_Count);


}

void InitGame()
{
	for (int i = 0; i < MAXPLAYERCOUNT; ++i) {
		Player_Info[i].fPosition_x = 0.0f;
		Player_Info[i].fPosition_y = -50.0f;
		Player_Info[i].fPosition_z = 0.0f;
	}


	Total_Cube.clear();
}

void CreateClientKeyInputThread(SOCKET& KeyInput_listen_sock)
{
	SOCKET* client_sock = new SOCKET();
	struct sockaddr_in clientaddr;

	// accept()
	int addrlen = sizeof(clientaddr);
	*client_sock = accept(KeyInput_listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
	if (*client_sock == INVALID_SOCKET) {
		err_display("CreateClientKeyInputThread() - accept()");
		return;
	}

	HANDLE hThread = CreateThread(NULL, 0, ProcessClientKeyInput,
		(LPVOID)client_sock, 0, NULL);
	if (hThread == NULL) {  closesocket(*client_sock); }
	else { CloseHandle(hThread); }
}

DWORD WINAPI ProcessClientKeyInput(LPVOID arg)
{
	printf("키 인풋 쓰레드 시작\n");
	SOCKET* ClientKeyInputSocket = (SOCKET*)arg;
	struct sockaddr_in clientaddr;

	struct KeyInput clientKeyInput{};
	int retval;
	while(1)
	{
		retval = recv(*ClientKeyInputSocket, (char*)&clientKeyInput, sizeof(clientKeyInput), 0);
		if (retval == SOCKET_ERROR||(clientKeyInput.Key<0 || clientKeyInput.Key>256)) {
			printf("?\n");
			break;
		}
		printf("%d 플레이어가 %d 버튼 ", clientKeyInput.PlayerNumber, clientKeyInput.Key);
		if(clientKeyInput.KeyDown)
		{
			clientKeyBuffer[clientKeyInput.PlayerNumber][clientKeyInput.Key] = true;
			printf("누름\n");


		}else
		{
			clientKeyBuffer[clientKeyInput.PlayerNumber][clientKeyInput.Key] = false;
			printf("뗌\n");
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

	HANDLE hThread = CreateThread(NULL, 0, SendPlayerDataToClient,
		(LPVOID)client_sock, 0, NULL);
	if (hThread == NULL) { closesocket(client_sock); }
	else { CloseHandle(hThread); }
}

DWORD WINAPI SendPlayerDataToClient(LPVOID arg)
{
	printf("플레이어 정보 전송 시작\n");
	SOCKET SendPlayerDataSocket = (SOCKET)arg;
	struct sockaddr_in clientaddr;

	
	
	int retval;
	while (1)
	{
		Sleep(30);
		Player_Info[0].fPosition_y += 0.05;
		Player_Info[1].fPosition_x -=0.05;
		Player_Info[2].fPosition_x +=0.05;
		Player_Info[2].fPosition_y +=0.05;
		retval = send(SendPlayerDataSocket, (char*)&Player_Info, sizeof(struct Player_Info) * MAXPLAYERCOUNT, 0);
		if (retval == SOCKET_ERROR ) {
			printf("?\n");
			break;
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
		retval = recv(CubeSocket, (char*)&clientCubeInput, sizeof(clientCubeInput), 0);
		if (retval == SOCKET_ERROR) {
			printf("?\n");
			break;
		}
		printf("Cube Position - %.2f, %.2f, %.2f", clientCubeInput.fPosition_x, clientCubeInput.fPosition_y, clientCubeInput.fPosition_z);
		printf("Cube Color - %.2f, %.2f, %.2f", clientCubeInput.fColor_r, clientCubeInput.fColor_g, clientCubeInput.fColor_b);

		// 여기서 큐브와 사람 충돌체크
		//..

		// 큐브 send to every cube
		for (auto i : socket_Cube_vector) {
			int retval = send(i, (char*)&clientCubeInput, sizeof(clientCubeInput), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				break;
			}
			std::cout << "Sending cube_info to the client" << std::endl;
		}
		
	}
	return 0;
}


// 타이머 콜백 함수
VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
	// 시간 업데이트
	remainingSeconds--;
	// 남은 시간이 0보다 크거나 같으면 클라이언트로 시간 업데이트 및 전송
	if (remainingSeconds >= 0) {
		Send_Game_Time();
	}
	else {
		// 게임 종료 이벤트 여기에 추가
		
		// 타이머 종료
		KillTimer(hwnd, TIMER_ID);
		for (auto i : socket_vector)	closesocket(i);		// 시간 소켓 close
	}
}


void Send_Game_Time() {
	// 시간 데이터 보내기
	for (auto i : socket_vector) {
		int retval = send(i, (char*)&remainingSeconds, sizeof(int), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
	}
	std::cout << "Sending time to the client: " << remainingSeconds << " seconds" << std::endl;
}

void SendPlayerLocationToAllClient()
{

}