#include "Common.h"
#include "Global.h"
#include "ServerData.h"
#include "ClientKeyInput.h"



// 함수
void ConnectAndAddPlayer(SOCKET&);
void InitGame();						// 게임 데이터 초기화 부분, 재시작 시 다시 호출하여 실행할 수 있도록 구현 예정
void CreateClientKeyInputThread(SOCKET&);
DWORD WINAPI ProcessClient(LPVOID arg); // 클라이언트와 데이터 통신
DWORD WINAPI ProcessClientKeyInput(LPVOID arg);

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

	// 플레이어 접속 체크 변수
	
	HANDLE hThread;


	// 게임 데이터 초기화
	InitGame();

	// 플레이어 지정한 수 인원 접속시키기
	while (Current_Player_Count != MAXPLAYERCOUNT) {
		ConnectAndAddPlayer(login_listen_sock);
		CreateClientKeyInputThread(KeyInput_listen_sock);
	}

	// 소켓 닫기
	closesocket(login_listen_sock);


	// TODO: 게임 시작 되므로 게임 시작 관련 스레드 제작 예정
	//// 스레드 생성 - 
	//hThread = CreateThread(NULL, 0, ProcessClient,
	//	(LPVOID)client_sock, 0, NULL);
	//if (hThread == NULL) { closesocket(client_sock); }
	//else { CloseHandle(hThread); }

	// 플레이어의 키 인풋 정보를 받는 쓰레드 생성

	// TODO: while문으로 main 쓰레드에서는 중력, 충돌체크 및 시간 전송 등이 진행되도록 구현 예정


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

	// 클라이언트 주소 변수에 추가 및 플레이어 수 증가
	clientAddr[Current_Player_Count] = clientaddr;
	Current_Player_Count += 1;

	// 새로운 플레이어 추가
	struct Player_Info* newPlayer = new struct Player_Info();
	Player_Info.push_back(newPlayer);

	// 접속한 클라이언트 정보 출력
	char addr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
	printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
		addr, ntohs(clientaddr.sin_port));

	printf("총 플레이어 수 : %d\n", Current_Player_Count);

}

void InitGame()
{
	Player_Info.clear();
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
		
		if(clientKeyInput.KeyDown)
		{
			printf("%d 눌림\n", clientKeyInput.Key);
		}else
		{
			printf("%d 뗌\n", clientKeyInput.Key);
		}
	}
	return 0;
}

DWORD WINAPI ProcessClient(LPVOID arg)
{
	printf("aaa\n");
	int retval;
	SOCKET client_sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;
	char buf[512 + 1];

	// 클라이언트 정보 얻기
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	while (1) {
		// 데이터 받기
		retval = recv(client_sock, buf, 512, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		// 받은 데이터 출력
		buf[retval] = '\0';
		printf("[TCP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), buf);

		// 데이터 보내기
		retval = send(client_sock, buf, retval, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
	}

	// 소켓 닫기
	closesocket(client_sock);
	printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
		addr, ntohs(clientaddr.sin_port));
	return 0;
}
